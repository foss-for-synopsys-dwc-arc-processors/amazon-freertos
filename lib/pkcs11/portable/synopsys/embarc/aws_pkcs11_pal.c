/*
 * Amazon FreeRTOS PKCS #11 PAL V1.0.0
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
 * @file aws_pkcs11_pal.c
 * @brief Device specific helpers for PKCS11 Interface.
 */

#include "embARC.h"
#include "embARC_debug.h"
#define PKCS11_CHECK_EXP(EXPR, ret, ERROR_CODE)  CHECK_EXP(EXPR, ret, ERROR_CODE, clean_exit)
/* Amazon FreeRTOS Includes. */
#include "aws_pkcs11.h"
#include "FreeRTOS.h"

/* C runtime includes. */
#include <stdio.h>
#include <string.h>

//aws_pkcs11_pal code without using SD card

#define STORAGE_SIZE    2048

typedef struct {
    uint32_t len;
    uint8_t data[STORAGE_SIZE];
}CERT_STORAGE;

CERT_STORAGE pkcs11palFILE_NAME_CLIENT_CERTIFICATE;
CERT_STORAGE pkcs11palFILE_NAME_KEY;
CERT_STORAGE pkcs11palFILE_CODE_SIGN_PUBLIC_KEY;

enum eObjectHandles
{
    eInvalidHandle = 0, /* According to PKCS #11 spec, 0 is never a valid object handle. */
    eAwsDevicePrivateKey = 1,
    eAwsDevicePublicKey,
    eAwsDeviceCertificate,
    eAwsCodeSigningKey
};

/* Converts a label to its respective filename and handle. */
static CK_OBJECT_HANDLE prvLabelToStorageHandle( uint8_t * pcLabel,
                               CERT_STORAGE ** pxStorage)
{

    if( pcLabel != NULL )
    {
        /* Translate from the PKCS#11 label to local storage file name. */
        if( 0 == memcmp( pcLabel,
                         &pkcs11configLABEL_DEVICE_CERTIFICATE_FOR_TLS,
                         sizeof( pkcs11configLABEL_DEVICE_CERTIFICATE_FOR_TLS ) ) )
        {
            *pxStorage = &pkcs11palFILE_NAME_CLIENT_CERTIFICATE;
            return eAwsDeviceCertificate;
        }
        else if( 0 == memcmp( pcLabel,
                              &pkcs11configLABEL_DEVICE_PRIVATE_KEY_FOR_TLS,
                              sizeof( pkcs11configLABEL_DEVICE_PRIVATE_KEY_FOR_TLS ) ) )
        {
            *pxStorage = &pkcs11palFILE_NAME_KEY;
            return eAwsDevicePrivateKey;
        }
        else if( 0 == memcmp( pcLabel,
                              &pkcs11configLABEL_DEVICE_PUBLIC_KEY_FOR_TLS,
                              sizeof( pkcs11configLABEL_DEVICE_PUBLIC_KEY_FOR_TLS ) ) )
        {
            *pxStorage = &pkcs11palFILE_NAME_KEY;
            return eAwsDevicePublicKey;
        }
        else if( 0 == memcmp( pcLabel,
                              &pkcs11configLABEL_CODE_VERIFICATION_KEY,
                              sizeof( pkcs11configLABEL_CODE_VERIFICATION_KEY ) ) )
        {
            *pxStorage = &pkcs11palFILE_CODE_SIGN_PUBLIC_KEY;
            return eAwsCodeSigningKey;
        }
    }
    *pxStorage = NULL;
    return eInvalidHandle;
}

/**
* @brief Writes a file to local storage.
*
* Port-specific file write for crytographic information.
*
* @param[in] pxLabel       Label of the object to be saved.
* @param[in] pucData       Data buffer to be written to file
* @param[in] ulDataSize    Size (in bytes) of data to be saved.
*
* @return The file handle of the object that was stored.
*/
CK_OBJECT_HANDLE PKCS11_PAL_SaveObject( CK_ATTRIBUTE_PTR pxLabel,
    uint8_t * pucData,
    uint32_t ulDataSize )
{
    CERT_STORAGE * pxStorage = NULL;
    CK_OBJECT_HANDLE xHandle = eInvalidHandle;
    xHandle = prvLabelToStorageHandle( pxLabel->pValue, &pxStorage);

    if( pxStorage != NULL )
    {
        if(ulDataSize > STORAGE_SIZE){
            configPRINTF( ( "PKCS11 storage size is not enough for %d bytes\r\n", ulDataSize ) );
            return eInvalidHandle;
        }
        memcpy( pxStorage->data, pucData, ulDataSize);
        pxStorage->len = ulDataSize;
    }

    return xHandle;
}

/**
* @brief Translates a PKCS #11 label into an object handle.
*
* Port-specific object handle retrieval.
*
*
* @param[in] pLabel         Pointer to the label of the object
*                           who's handle should be found.
* @param[in] usLength       The length of the label, in bytes.
*
* @return The object handle if operation was successful.
* Returns eInvalidHandle if unsuccessful.
*/
CK_OBJECT_HANDLE PKCS11_PAL_FindObject( uint8_t * pLabel,
    uint8_t usLength )
{
    /* Avoid compiler warnings about unused variables. */
    ( void ) usLength;

    CK_OBJECT_HANDLE xHandle = eInvalidHandle;
    CERT_STORAGE * pxStorage = NULL;
    // FATFS *fs;

    /* Converts a label to its respective filename and handle. */
   xHandle = prvLabelToStorageHandle( pLabel, &pxStorage);

    /* Check if object exists/has been created before returning. */
    if( pxStorage == NULL )
    {
        return eInvalidHandle;
    }
    // else if( pxStorage->len == 0 )
    // {
    //     return eInvalidHandle;
    // }

    return xHandle;
}

/**
* @brief Gets the value of an object in storage, by handle.
*
* Port-specific file access for cryptographic information.
*
* This call dynamically allocates the buffer which object value
* data is copied into.  PKCS11_PAL_GetObjectValueCleanup()
* should be called after each use to free the dynamically allocated
* buffer.
*
* @sa PKCS11_PAL_GetObjectValueCleanup
*
* @param[in] pcFileName    The name of the file to be read.
* @param[out] ppucData     Pointer to buffer for file data.
* @param[out] pulDataSize  Size (in bytes) of data located in file.
* @param[out] pIsPrivate   Boolean indicating if value is private (CK_TRUE)
*                          or exportable (CK_FALSE)
*
* @return CKR_OK if operation was successful.  CKR_KEY_HANDLE_INVALID if
* no such object handle was found, CKR_DEVICE_MEMORY if memory for
* buffer could not be allocated, CKR_FUNCTION_FAILED for device driver
* error.
*/
CK_RV PKCS11_PAL_GetObjectValue( CK_OBJECT_HANDLE xHandle,
    uint8_t ** ppucData,
    uint32_t * pulDataSize,
    CK_BBOOL * pIsPrivate )
{
    CK_RV ulReturn = CKR_OK;
    CERT_STORAGE * pxStorage = NULL;
    // FATFS *fs;


    if( xHandle == eAwsDeviceCertificate )
    {
        pxStorage = &pkcs11palFILE_NAME_CLIENT_CERTIFICATE;
        *pIsPrivate = CK_FALSE;
    }
    else if( xHandle == eAwsDevicePrivateKey )
    {
        pxStorage = &pkcs11palFILE_NAME_KEY;
        *pIsPrivate = CK_TRUE;
    }
    else if( xHandle == eAwsDevicePublicKey )
    {
        /* Public and private key are stored together in same file. */
        pxStorage = &pkcs11palFILE_NAME_KEY;
        *pIsPrivate = CK_FALSE;
    }
    else if( xHandle == eAwsCodeSigningKey )
    {
        pxStorage = &pkcs11palFILE_CODE_SIGN_PUBLIC_KEY;
        *pIsPrivate = CK_FALSE;
    }
    else
    {
        ulReturn = CKR_KEY_HANDLE_INVALID;
    }

    if( pxStorage != NULL )
    {
        /* Get the file size. */
        *pulDataSize = pxStorage->len;
        /* Create a buffer. */
        *ppucData = pvPortMalloc( *pulDataSize );
        PKCS11_CHECK_EXP(*ppucData != NULL, ulReturn, CKR_DEVICE_MEMORY);

        memcpy(*ppucData, pxStorage->data, *pulDataSize);
    }

clean_exit:
    /* Clean up. */
    // if( pxStorage != NULL ) {
        // vPortFree(fs);
    // }
    return ulReturn;
}


/**
* @brief Cleanup after PKCS11_GetObjectValue().
*
* @param[in] pucData       The buffer to free.
*                          (*ppucData from PKCS11_PAL_GetObjectValue())
* @param[in] ulDataSize    The length of the buffer to free.
*                          (*pulDataSize from PKCS11_PAL_GetObjectValue())
*/
void PKCS11_PAL_GetObjectValueCleanup( uint8_t * pucData,
    uint32_t ulDataSize )
{
    /* Unused parameters. */
    ( void ) ulDataSize;

    if( pucData != NULL )
    {
        vPortFree( pucData );
    }
}

/*-----------------------------------------------------------*/

int mbedtls_hardware_poll( void * data,
                           unsigned char * output,
                           size_t len,
                           size_t * olen )
{
#if defined (BOARD_TRNG_ID)
    DEV_TRNG_PTR trng = dw_trng_get_dev(BOARD_TRNG_ID);
    uint32_t buffer[4];
    (( void ) data );

    if(trng == NULL){
        return 0;
    }
    trng->trng_open();

    int nChunks = len / sizeof( buffer );
    int nLeft = len % sizeof( buffer );

    while( nChunks-- )
    {
        trng->trng_read(buffer);
        memcpy( output, buffer, sizeof( buffer ) );
        output += sizeof( buffer );
    }

    if( nLeft )
    {
        trng->trng_read(buffer);
        memcpy( output, buffer, nLeft );
    }

    trng->trng_close();
    *olen = len;
    return 0;
#else
/* use sw to do hardware poll */
    *olen = len;
    return 0;
#endif
}
