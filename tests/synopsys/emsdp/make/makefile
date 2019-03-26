# Application name
APPL := aws_tests

BOARD ?= emsdp

ifeq ($(BOARD), emsdp)
EXT_DEV_LIST += wifi/rs9113
else
EXT_DEV_LIST += wifi/mrf24g
endif

#
# root dir of embARC
#
ifndef AWS_FREERTOS_ROOT
export AWS_FREERTOS_ROOT := ../../../..
endif

ifndef EMBARC_ROOT
export EMBARC_ROOT := $(AWS_FREERTOS_ROOT)/lib/third_party/mcu_vendor/synopsys/embARC
endif

# Selected OS
OS_SEL ?= aws_freertos
# Select Middleware Packages
MID_SEL = common lwip-contrib

AWS_FREERTOS_LIB_SEL = unity tests utils wifi lwip crypto pkcs11 tls mbedtls bufferpool mqtt

# application source dirs
APPL_CSRC_DIR = .
APPL_ASMSRC_DIR = .

# application include dirs
APPL_INC_DIR = .

# application defines
APPL_DEFINES = -DAMAZON_FREERTOS_ENABLE_UNIT_TESTS -DUNITY_INCLUDE_CONFIG_H -DMID_LWIP

# include current project makefile
COMMON_COMPILE_PREREQUISITES += makefile

### Options above must be added before include options.mk ###
# include key embARC build system makefile
include $(EMBARC_ROOT)/options/options.mk
