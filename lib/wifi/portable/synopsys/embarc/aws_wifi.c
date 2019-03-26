/*
 * Amazon FreeRTOS Wi-Fi V1.0.0
 * Copyright (C) 2018 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */

/**
 * @file aws_wifi.c
 * @brief Wi-Fi Interface.
 */
#include "embARC.h"
#define DBG_LESS
#include "embARC_debug.h"
#define WIFI_CHECK_EXP(EXPR, ERROR_CODE)  CHECK_EXP(EXPR, ercd, ERROR_CODE, error_exit)
/* Socket and Wi-Fi interface includes. */
#include "FreeRTOS.h"
#include "aws_wifi.h"
#include "lwip_wifi.h"
#include "lwip/dhcp.h"
#include "lwip/netdb.h"

/* Wi-Fi configuration includes. */
#include "aws_wifi_config.h"

static SemaphoreHandle_t wifi_serialize_mutex = NULL;
static const TickType_t xMaxSemaphoreBlockTime = pdMS_TO_TICKS( 20000UL );

#define WIFI_DELAY_MIN_TIME_MS  1000
#define MAX_RETRY_NUM   4

// wifi module specific variables & macros & functions
#ifdef WIFI_MRF24G

static TaskHandle_t task_handle_wifi;

#define TASK_STACK_SIZE_WIFI    1024
#define TASK_WIFI_PERIOD        50
#define TASK_PRI_WIFI       (configMAX_PRIORITIES-1)

static void task_wifi(void *par)
{
    DEV_WNIC_PTR wifi_wnic;

    wifi_wnic = (DEV_WNIC_PTR) par;

    while (1) {
        wifi_wnic->period_process(par);
        vTaskDelay(TASK_WIFI_PERIOD);
    }
}
#endif


/*-----------------------------------------------------------*/
static uint8_t wifi_is_init = 0;
static uint8_t wifi_init_once = 0;
WIFIReturnCode_t WIFI_On( void )
{
    int32_t ercd = E_OK;
    DEV_WNIC_PTR wifi_wnic;

    wifi_wnic = wnic_get_dev(BOARD_PMWIFI_0_ID);
    WIFI_CHECK_EXP(wifi_wnic != NULL, E_OBJ);

    wifi_serialize_mutex = xSemaphoreCreateMutex();
    if(wifi_init_once == 0){
        lwip_wifi_init();
#ifdef WIFI_MRF24G
        xTaskCreate((TaskFunction_t)task_wifi, "wifi-conn", TASK_STACK_SIZE_WIFI,
            (void *)wifi_wnic, TASK_PRI_WIFI, &task_handle_wifi);

        while (wifi_wnic->poll_init_status() == WNIC_DURING_INITIALIZATION ||
              wifi_wnic->poll_init_status() == WNIC_NOT_INITIALIZED) {
            vTaskDelay(TASK_WIFI_PERIOD * 2);
        }

        if (wifi_wnic->poll_init_status()  != WNIC_INIT_SUCCESSFUL) {
            return eWiFiFailure;
        }
#endif
        wifi_init_once = 1;
        wifi_is_init = 1;
    } else {
        // Obtain the wifi_serial mutex.
        if(xSemaphoreTake( wifi_serialize_mutex, xMaxSemaphoreBlockTime) == pdTRUE) {
            if(wifi_is_init == 1 ){
                ercd = wifi_wnic->wnic_reset();
            } else {
                ercd = wifi_wnic->wnic_init(WNIC_NETWORK_TYPE_INFRASTRUCTURE);
                if(ercd == E_OK){
                    wifi_is_init = 1;
                }
            }
            // Give back the wifi_serial mutex.
            xSemaphoreGive(wifi_serialize_mutex);
            WIFI_CHECK_EXP(ercd == E_OK, ercd);
        } else {
            dbg_printf(DBG_MORE_INFO, "[%s] could not get mutex\r\n", __FUNCTION__);
            return eWiFiFailure;
        }
    }
    return eWiFiSuccess;
error_exit:
    wifi_is_init = 0;
    dbg_printf(DBG_MORE_INFO, "[%s] return %d\r\n", __FUNCTION__, ercd);
    return eWiFiFailure;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_Off( void )
{
    vSemaphoreDelete(wifi_serialize_mutex);
    return eWiFiSuccess;
}
/*-----------------------------------------------------------*/

#define COND_NO_AP          (0x3)
#define COND_REJOIN_FAIN    (0x19)
#define COND_AUTH_TIMEOUT   (0x48)
#define COND_ASSOCI_TIMEOUT (0x49)

static inline int32_t _is_retry_condition(int32_t condition){
    switch(condition){
        case COND_NO_AP:
        case COND_REJOIN_FAIN:
        case COND_AUTH_TIMEOUT:
        case COND_ASSOCI_TIMEOUT:
            return 1;
        default: return 0;
    }
}

static int32_t _wifi_connect_need_retry(int32_t condition){
    static int32_t retry_count = MAX_RETRY_NUM;

    if(retry_count == 0){
        retry_count = MAX_RETRY_NUM;
        return 0;
    }
    if(_is_retry_condition(condition)){
        retry_count--;
        dbg_printf(DBG_MORE_INFO, "retry\r\n");
        return 1;
    }
    return 0;
}

WIFIReturnCode_t WIFI_ConnectAP( const WIFINetworkParams_t * const pxNetworkParams )
{
    int32_t ercd = E_OK;
    uint8_t security_type;
    WNIC_AUTH_KEY auth_key;
    DEV_WNIC_PTR wifi_wnic;
    wifi_wnic = wnic_get_dev(BOARD_PMWIFI_0_ID);
    uint32_t delay_time = WIFI_DELAY_MIN_TIME_MS;
    WIFI_CHECK_EXP(wifi_wnic != NULL, E_OBJ);
    WIFI_CHECK_EXP(pxNetworkParams != NULL, E_PAR);
    WIFI_CHECK_EXP(pxNetworkParams->pcSSID != NULL, E_PAR);
    if(pxNetworkParams->xSecurity != eWiFiSecurityOpen )
    {
        WIFI_CHECK_EXP(pxNetworkParams->pcPassword != NULL, E_PAR);
        security_type = AUTH_SECURITY_WPA_AUTO_WITH_PASS_PHRASE;
        auth_key.key = (const uint8_t *)pxNetworkParams->pcPassword;
        auth_key.key_len = pxNetworkParams->ucPasswordLength;
        auth_key.key_idx = 0;
    } else {
        security_type = AUTH_SECURITY_OPEN;
        auth_key.key = NULL;
        auth_key.key_len = 0;
        auth_key.key_idx = 0;
    }
    dbg_printf(DBG_MORE_INFO, "Connect WIFI: %s\r\n", pxNetworkParams->pcSSID);
    board_delay_ms(1, 1);
    //Retry if no AP found
    do {
        // Obtain the wifi_serial mutex.
        if(xSemaphoreTake( wifi_serialize_mutex, xMaxSemaphoreBlockTime) == pdTRUE) {
            ercd = wifi_wnic->wnic_connect(security_type, (const uint8_t *)pxNetworkParams->pcSSID, &auth_key);
            // Give back the wifi_serial mutex.
            xSemaphoreGive(wifi_serialize_mutex);
        } else {
            dbg_printf(DBG_MORE_INFO, "[%s] could not get mutex\r\n", __FUNCTION__);
            //not connected, no clean-up needed
            return eWiFiFailure;
        }
        if(ercd == E_OK){
#ifdef WIFI_MRF24G
            while (wifi_wnic->poll_conn_status() == WNIC_DURING_CONNECTION ||
                wifi_wnic->poll_conn_status() == WNIC_CONN_TEMP_LOST) {
                /* needs some time to connect */
                board_delay_ms(WIFI_DELAY_MIN_TIME_MS, 1);
            }
            if (wifi_wnic->poll_conn_status() != WNIC_CONNECTED) {
                return eWiFiFailure;
            }
#endif

            // make sure dhcp is completed before return
            while(!lwip_wifi_isup()){
                board_delay_ms(1000, 1);
            }

            return eWiFiSuccess;
        }
        // Delay before retry
        board_delay_ms(delay_time, 1);
        // Exponetially backoff the retry times, arbitrarily chose 2
        delay_time *= 2;
    } while (_wifi_connect_need_retry(ercd));

error_exit:
    WIFI_Disconnect();
    dbg_printf(DBG_MORE_INFO, "[%s] return %d\r\n", __FUNCTION__, ercd);
    return eWiFiFailure;
}

/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_Disconnect( void )
{
    int32_t ercd = E_OK;
    DEV_WNIC_PTR wifi_wnic;
    wifi_wnic = wnic_get_dev(BOARD_PMWIFI_0_ID);
    WIFI_CHECK_EXP(wifi_wnic != NULL, E_OBJ);
    // Obtain the wifi_serial mutex.
    if(xSemaphoreTake( wifi_serialize_mutex, xMaxSemaphoreBlockTime) == pdTRUE) {
        ercd = wifi_wnic->wnic_disconnect();
        // Give back the wifi_serial mutex.
        xSemaphoreGive(wifi_serialize_mutex);
    } else {
        dbg_printf(DBG_MORE_INFO, "[%s] could not get mutex\r\n", __FUNCTION__);
        return eWiFiFailure;
    }
    WIFI_CHECK_EXP(ercd == E_OK, ercd);
#ifdef WIFI_MRF24G
    while (wifi_wnic->poll_conn_status() == WNIC_DURING_DISCONNECTION) {
        vTaskDelay(TASK_WIFI_PERIOD * 2);
    }
    if (wifi_wnic->poll_conn_status() != WNIC_NOT_CONNECTED) {
        return eWiFiFailure;
    }
#endif
    return eWiFiSuccess;
error_exit:
    dbg_printf(DBG_MORE_INFO, "[%s] return %d\r\n", __FUNCTION__, ercd);
    return eWiFiFailure;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_Reset( void )
{
    int32_t ercd = E_OK;
    DEV_WNIC_PTR wifi_wnic;
    wifi_wnic = wnic_get_dev(BOARD_PMWIFI_0_ID);
    WIFI_CHECK_EXP(wifi_wnic != NULL, E_OBJ);
    // Obtain the wifi_serial mutex.
    if(xSemaphoreTake( wifi_serialize_mutex, xMaxSemaphoreBlockTime) == pdTRUE) {
        ercd = wifi_wnic->wnic_reset();
        // Give back the wifi_serial mutex.
        xSemaphoreGive(wifi_serialize_mutex);
    } else {
        dbg_printf(DBG_MORE_INFO, "[%s] could not get mutex\r\n", __FUNCTION__);
        wifi_is_init = 0;
        return eWiFiFailure;
    }
    WIFI_CHECK_EXP(ercd == E_OK, ercd);
    return eWiFiSuccess;
error_exit:
    wifi_is_init = 0;
    dbg_printf(DBG_MORE_INFO, "[%s] return %d\r\n", __FUNCTION__, ercd);
    return eWiFiFailure;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_Scan( WIFIScanResult_t * pxBuffer,
                            uint8_t ucNumNetworks )
{
    int32_t ercd = E_OK;
    DEV_WNIC_PTR wifi_wnic;
    WIFI_CHECK_EXP(pxBuffer != NULL, E_PAR);
    WIFI_CHECK_EXP(ucNumNetworks != 0, E_PAR);

    wifi_wnic = wnic_get_dev(BOARD_PMWIFI_0_ID);
    WIFI_CHECK_EXP(wifi_wnic != NULL, E_OBJ);
    ercd = wifi_wnic->start_scan();
    dbg_printf(DBG_MORE_INFO, "[%s] start_scan return %d\r\n", __FUNCTION__, ercd);
    WIFI_CHECK_EXP(ercd == E_OK, ercd);
#if WIFI_MRF24G
    while(wifi_wnic->poll_scan_status() == WNIC_DURING_SCAN) {
        vTaskDelay(TASK_WIFI_PERIOD * 2);
    }
#endif
    int32_t rsi_scan_rst_cnt = wifi_wnic->get_scan_result_cnt();
    rsi_scan_rst_cnt = ucNumNetworks < rsi_scan_rst_cnt ? ucNumNetworks :rsi_scan_rst_cnt;
    dbg_printf(DBG_MORE_INFO, "[%s] get_scan_result_cnt return %d\r\n", __FUNCTION__, ucNumNetworks);
    for (int8_t i = 0 ; i < rsi_scan_rst_cnt; i++){
        WNIC_SCAN_RESULT result;
        ercd = wifi_wnic->get_scan_result(i, &result);
        WIFI_CHECK_EXP(ercd == E_OK, ercd);
        memcpy((pxBuffer+i)->cSSID, result.ssid, result.ssidlen);
        memcpy((pxBuffer+i)->ucBSSID, result.bssid, WNIC_BSSID_MAX_LEN);
        (pxBuffer+i)->cRSSI = -result.rssi;
        (pxBuffer+i)->cChannel = result.channel;
        (pxBuffer+i)->ucHidden = 0;
        if(result.ap_config.ap_cfg.privacy == 1){
            (pxBuffer+i)->xSecurity = eWiFiSecurityOpen;
        } else if(result.ap_config.ap_cfg.wpa == 1){
            (pxBuffer+i)->xSecurity = eWiFiSecurityWPA;
        } else if(result.ap_config.ap_cfg.wpa2 == 1){
            (pxBuffer+i)->xSecurity = eWiFiSecurityWPA2;
        } else {
            (pxBuffer+i)->xSecurity = eWiFiSecurityNotSupported;
        }
    }
    ercd = wifi_wnic->stop_scan();
    WIFI_CHECK_EXP(ercd == E_OK, ercd);
    return eWiFiSuccess;
error_exit:
    dbg_printf(DBG_MORE_INFO, "[%s] return %d\r\n", __FUNCTION__, ercd);
    return eWiFiFailure;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_SetMode( WIFIDeviceMode_t xDeviceMode )
{
    /* FIX ME. */
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_GetMode( WIFIDeviceMode_t * pxDeviceMode )
{
    /* FIX ME. */
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_NetworkAdd( const WIFINetworkProfile_t * const pxNetworkProfile,
                                  uint16_t * pusIndex )
{
    /* FIX ME. */
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_NetworkGet( WIFINetworkProfile_t * pxNetworkProfile,
                                  uint16_t usIndex )
{
    /* FIX ME. */
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_NetworkDelete( uint16_t usIndex )
{
    /* FIX ME. */
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_Ping( uint8_t * pucIPAddr,
                            uint16_t usCount,
                            uint32_t ulIntervalMS )
{
    /* FIX ME. */
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_GetIP( uint8_t * pucIPAddr )
{
    ip4_addr_t      ip4_addr;
    struct netif    *iface;

    if( NULL == pucIPAddr || NULL == ( iface = netif_find( "rs1" ) ) )
    {
        return eWiFiFailure;
    }

    if ( dhcp_supplied_address( iface ) )
    {
        struct dhcp *d = netif_dhcp_data(iface);

        ip4_addr = d->offered_ip_addr;
    }
    else
    {
        ip4_addr = iface->ip_addr;
    }

    memcpy( pucIPAddr, &ip4_addr.addr, sizeof( ip4_addr.addr ) );

    return eWiFiSuccess;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_GetMAC( uint8_t * pucMac )
{
    int32_t ercd = E_OK;
    DEV_WNIC_PTR wifi_wnic;
    wifi_wnic = wnic_get_dev(BOARD_PMWIFI_0_ID);
    // Obtain the wifi_serial mutex.
    if(xSemaphoreTake( wifi_serialize_mutex, xMaxSemaphoreBlockTime) == pdTRUE) {
        ercd = wifi_wnic->get_macaddr(pucMac);
        // Give back the wifi_serial mutex.
        xSemaphoreGive(wifi_serialize_mutex);
    } else {
        dbg_printf(DBG_MORE_INFO, "[%s] could not get mutex\r\n", __FUNCTION__);
        wifi_is_init = 0;
        return eWiFiFailure;
    }
    return ercd == E_OK ? eWiFiSuccess : eWiFiFailure;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_GetHostIP( char * pcHost,
                                 uint8_t * pucIPAddr )
{
    int                 s;
    struct addrinfo     hints;
    struct addrinfo     *result;
    struct sockaddr_in  *addr_in;

    if( NULL == pcHost || NULL == pucIPAddr )
    {
        return eWiFiFailure;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family     = AF_INET;
    hints.ai_socktype   = SOCK_DGRAM;
    hints.ai_flags      = 0;
    hints.ai_protocol   = 0;

    s = getaddrinfo( pcHost, NULL, &hints, &result );
    if( s != 0 )
    {
        return eWiFiFailure;
    }

    if( result )
    {
        /*
         * ai_addr is of struct sockaddr type.
         *
            struct sockaddr {
              u8_t        sa_len;
              sa_family_t sa_family;
              char        sa_data[14];
            };
         */

        addr_in = (struct sockaddr_in *)result->ai_addr;

        memcpy( pucIPAddr, &addr_in->sin_addr.s_addr,
                sizeof( addr_in->sin_addr.s_addr ) );
        dbg_printf(DBG_MORE_INFO, "[%s]:[%s]->[%d.%d.%d.%d]\r\n", __FUNCTION__, pcHost, pucIPAddr[0], pucIPAddr[1], pucIPAddr[2], pucIPAddr[3]);
    }
    else
    {
        return eWiFiFailure;
    }

    freeaddrinfo( result );

    return eWiFiSuccess;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_StartAP( void )
{
    /* FIX ME. */
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_StopAP( void )
{
    /* FIX ME. */
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_ConfigureAP( const WIFINetworkParams_t * const pxNetworkParams )
{
    /* FIX ME. */
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_SetPMMode( WIFIPMMode_t xPMModeType,
                                 const void * pvOptionValue )
{
    /* FIX ME. */
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_GetPMMode( WIFIPMMode_t * pxPMModeType,
                                 void * pvOptionValue )
{
    /* FIX ME. */
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

BaseType_t WIFI_IsConnected(void)
{
    int32_t ercd = E_OK;
    DEV_WNIC_PTR wifi_wnic;
    wifi_wnic = wnic_get_dev(BOARD_PMWIFI_0_ID);
    WIFI_CHECK_EXP(wifi_wnic != NULL, E_OBJ);
    ercd = wifi_wnic->poll_conn_status();
    WIFI_CHECK_EXP(ercd == WNIC_CONNECTED, ercd);
    return pdTRUE;
error_exit:
    dbg_printf(DBG_MORE_INFO, "[%s] return %d\r\n", __FUNCTION__, ercd);
    return pdFALSE;
}
