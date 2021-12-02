#*******************************************************************************
#   (c) 2017 - 2021 - hirish - vekexasia
#   (c) 2016 Ledger
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#*******************************************************************************

ifeq ($(BOLOS_SDK),)
$(error BOLOS_SDK is not set)
endif
ifeq (customCA.key,$(wildcard customCA.key))
  SCP_PRIVKEY=`cat customCA.key`
endif
include $(BOLOS_SDK)/Makefile.defines

APPVERSION = 2.0.2
APP_LOAD_PARAMS = --targetVersion "" --curve ed25519 $(COMMON_LOAD_PARAMS)
ifeq ($(TARGET_NAME),TARGET_NANOX)
APP_LOAD_PARAMS += --appFlags 0x240 # with BLE support
else
APP_LOAD_PARAMS += --appFlags 0x40
endif

# Main app configuration

APPNAME = "Lisk 2.0 Dev"
APP_LOAD_PARAMS += --path "44'/134'"
LEGACY_ADDRESS_SUFFIX = "L"
LEGACY_ADDRESS_SUFFIX_LENGTH = 1
LISK32_ADDRESS_PREFIX = "lsk"
LISK32_ADDRESS_PREFIX_LENGTH = 3
# Pipes will be translated as spaces when defining cc_cmdline
SIGNED_MESSAGE_PREFIX := "Lisk|Signed|Message:\n"
NVRAM_MAX = 0

$(info APPNAME=$(APPNAME) SIGNED_MESSAGE_PREFIX=$(SIGNED_MESSAGE_PREFIX))

# Build configuration

APP_SOURCE_PATH += src
SDK_SOURCE_PATH += lib_u2f lib_stusb lib_stusb_impl lib_ux

ifeq ($(TARGET_NAME),TARGET_NANOX)
SDK_SOURCE_PATH += lib_blewbxx lib_blewbxx_impl
endif

DEFINES   += OS_IO_SEPROXYHAL IO_SEPROXYHAL_BUFFER_SIZE_B=300
DEFINES   += HAVE_BAGL HAVE_SPRINTF
#DEFINES   += HAVE_PRINTF PRINTF=screen_printf
DEFINES   += PRINTF\(...\)=
DEFINES   += HAVE_IO_USB HAVE_L4_USBLIB IO_USB_MAX_ENDPOINTS=4 IO_HID_EP_LENGTH=64 HAVE_USB_APDU
DEFINES   += APP_MAJOR_VERSION=$(APPVERSION_M) APP_MINOR_VERSION=$(APPVERSION_N) APP_PATCH_VERSION=$(APPVERSION_P)
DEFINES   += MAX_ADPU_OUTPUT_SIZE=$(MAX_ADPU_OUTPUT_SIZE)

# U2F
DEFINES   += HAVE_IO_U2F
DEFINES   += U2F_PROXY_MAGIC=\"hirish\"

#WEBUSB_URL = www.ledgerwallet.com
#DEFINES   += HAVE_WEBUSB WEBUSB_URL_SIZE_B=$(shell echo -n $(WEBUSB_URL) | wc -c) WEBUSB_URL=$(shell echo -n $(WEBUSB_URL) | sed -e "s/./\\\'\0\\\',/g")
DEFINES   += HAVE_WEBUSB WEBUSB_URL_SIZE_B=0 WEBUSB_URL=""

DEFINES   += USB_SEGMENT_SIZE=64
DEFINES   += BLE_SEGMENT_SIZE=32 #max MTU, min 20
DEFINES   += U2F_REQUEST_TIMEOUT=10000 # 10 seconds
DEFINES   += UNUSED\(x\)=\(void\)x
DEFINES   += APPVERSION=\"$(APPVERSION)\"
DEFINES   += APPNAME=\"$(APPNAME)\"
DEFINES   += COINID=lisk
DEFINES   += COINIDSTR=\"lisk\"
DEFINES   += LEGACY_ADDRESS_SUFFIX=\"$(LEGACY_ADDRESS_SUFFIX)\"
DEFINES   += LEGACY_ADDRESS_SUFFIX_LENGTH=$(LEGACY_ADDRESS_SUFFIX_LENGTH)
DEFINES   += LISK32_ADDRESS_PREFIX=\"$(LISK32_ADDRESS_PREFIX)\"
DEFINES   += LISK32_ADDRESS_PREFIX_LENGTH=$(LISK32_ADDRESS_PREFIX_LENGTH)
DEFINES   += SIGNED_MESSAGE_PREFIX=\"$(SIGNED_MESSAGE_PREFIX)\"
DEFINES   += NVRAM_MAX=$(NVRAM_MAX)
DEFINES   += HAVE_UX_FLOW

ifeq ($(TARGET_NAME),TARGET_NANOX)
DEFINES   += HAVE_GLO096
DEFINES   += HAVE_BAGL BAGL_WIDTH=128 BAGL_HEIGHT=64
DEFINES   += HAVE_BAGL_ELLIPSIS # long label truncation feature
DEFINES   += HAVE_BAGL_FONT_OPEN_SANS_REGULAR_11PX
DEFINES   += HAVE_BAGL_FONT_OPEN_SANS_EXTRABOLD_11PX
DEFINES   += HAVE_BAGL_FONT_OPEN_SANS_LIGHT_16PX

DEFINES   += HAVE_BLE BLE_COMMAND_TIMEOUT_MS=2000
DEFINES   += HAVE_BLE_APDU # basic ledger apdu transport over BLE
endif

ifeq ($(TARGET_NAME),TARGET_NANOX)
ICONNAME=nanox_lisk.gif
else
ICONNAME=nanos_lisk.gif
endif

# Pending security review
APP_LOAD_PARAMS += --tlvraw 9F:01
DEFINES += HAVE_PENDING_REVIEW_SCREEN

# Compiler, assembler, and linker

ifneq ($(BOLOS_ENV),)
$(info BOLOS_ENV=$(BOLOS_ENV))
CLANGPATH := $(BOLOS_ENV)/clang-arm-fropi/bin/
GCCPATH := $(BOLOS_ENV)/gcc-arm-none-eabi/bin/
else
$(info BOLOS_ENV is not set: falling back to CLANGPATH and GCCPATH)
endif
ifeq ($(CLANGPATH),)
$(info CLANGPATH is not set: clang will be used from PATH)
endif
ifeq ($(GCCPATH),)
$(info GCCPATH is not set: arm-none-eabi-* will be used from PATH)
endif

CC := $(CLANGPATH)clang
CFLAGS += -O3 -Os

AS := $(GCCPATH)arm-none-eabi-gcc
AFLAGS +=

LD := $(GCCPATH)arm-none-eabi-gcc
LDFLAGS += -O3 -Os
LDLIBS += -lm -lgcc -lc

# import rules to compile glyphs(/pone)
include $(BOLOS_SDK)/Makefile.glyphs

# Main rules

all: default

load: all
	python -m ledgerblue.loadApp $(APP_LOAD_PARAMS)

delete:
	python -m ledgerblue.deleteApp $(COMMON_DELETE_PARAMS)

# Import generic rules from the SDK

include $(BOLOS_SDK)/Makefile.rules
cc_cmdline = $(CC) -c $(CFLAGS) $(subst |, ,$(addprefix -D,$(2))) $(addprefix -I,$(1)) -o $(4) $(3)

listvariants:
	@echo VARIANTS COIN lisk
