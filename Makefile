##########################################################################################################################
# File automatically-generated by tool: [projectgenerator] version: [4.2.0-B44] date: [Wed Jan 24 17:11:25 CET 2024] 
##########################################################################################################################

# ------------------------------------------------
# Generic Makefile (based on gcc)
#
# ChangeLog :
#	2017-02-10 - Several enhancements + project update mode
#   2015-07-22 - first version
# ------------------------------------------------

current_makefile := $(firstword $(MAKEFILE_LIST))
current_repo_status := $(shell  if !(git status --porcelain); then echo ''; fi)
current_repo_commit := $(shell  if !(git log -1 --format="%h"); then echo 'unknow'; fi)

######################################
# target
######################################
TARGET = op-tracker-firmware


######################################
# building variables
######################################
# debug build?
DEBUG = 1
VERBOSE = 1
USE_BAREMETAL = 1

# Select APPlication. Can be:
# * STDLN: for the standalone application sending one message at startup
# * GUI: for the application using the AT commands from UART link
# * TRACKER: for the OP Tracker application (TRACKER mode use custom MAC_PRFL)
APP = TRACKER

# Select output port
COMM = UART

# Select Kineis stack MAC profile. Can be:
# * BASIC: basic profile, sending message once immediately 
# * BLIND: blind profile, sending message sevral times periodically 
MAC_PRFL = BLIND

# LPM: depest low power mode supported can be:
# NONE, SLEEP, STOP, STANDBY, SHUTDOWN
LPM = SLEEP

# * KRD board: choose between: KRD_FW_LP, KRD_FW_MP
KRD_BOARD = KRD_FW_MP

# optimization
ifeq ($(DEBUG), 1)
OPT = -Og
else
OPT = -O1
endif

#######################################
# Kineis related
#######################################
KINEIS_DIR = Kineis
KINEIS_VERSION = v1.0.0

# Build path
BUILD_DIR = build
BUILD_INFO_FILE = build_info.c
BUILD_VERSION :=
BUILD_DATE = $(shell date +"%b %d %Y_%H:%M:%S")

# doc/doxygen related
DOC_DIR = $(KINEIS_DIR)/Doc/krd_fw
DOXY_WARN_LOGFILE = $(KINEIS_DIR)/doxy_warn_log_file.txt

# Get Library versions from listed sources
LIB_INFO_SOURCES = \
$(KINEIS_DIR)/Lib/libkineis_info.c \
$(KINEIS_DIR)/Lib/libknsrf_wl_info.c

#LIB_VERSIONS := $(foreach file, $(LIB_INFO_SOURCES), $(shell   if !(cat $(file) | grep "$(notdir $(file:.c=))\[\]" | sed  's/.*"\(.*\)".*/\1/'); then echo 'serach_$(notdir $(file:.c=))_variable'; fi))
LIB_VERSIONS := $(foreach file, $(LIB_INFO_SOURCES), $(shell cat $(file) | grep "$(notdir $(file:.c=))\[\]" | sed  's/.*"\(.*\)".*/\1/'))  
LIB_VERSIONS := $(shell echo $(LIB_VERSIONS)  | sed  's/ /,/g')

#######################################
# includes
#######################################
# include *.mk makefiles here if needed as impacted by variables above
#include kineis_sw/Make/libknsrf_wl.mk

######################################
# source
######################################
# C sources
C_SOURCES = \
Core/Src/main.c \
Core/Src/stm32wlxx_it.c \
Core/Src/stm32wlxx_hal_msp.c \
Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_rcc.c \
Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_rcc_ex.c \
Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_flash.c \
Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_flash_ex.c \
Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_gpio.c \
Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_dma.c \
Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_dma_ex.c \
Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_pwr.c \
Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_pwr_ex.c \
Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_cortex.c \
Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal.c \
Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_exti.c \
Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_subghz.c \
Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_tim.c \
Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_tim_ex.c \
Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_uart.c \
Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_uart_ex.c \
Core/Src/system_stm32wlxx.c \
Core/Src/gpio.c \
Core/Src/syscalls.c \
Core/Src/usart.c \
Core/Src/subghz.c \
Core/Src/tim.c \
Core/Src/rtc.c \
Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_rtc.c \
Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_rtc_ex.c 

C_SOURCES += \
$(KINEIS_DIR)/Extdep/Conf/kns_assert.c \
$(KINEIS_DIR)/Extdep/Conf/kns_cs.c \
$(KINEIS_DIR)/Extdep/Conf/kns_q_conf.c \
$(KINEIS_DIR)/Extdep/MGR_LOG/Src/mgr_log.c \
$(KINEIS_DIR)/Extdep/MGR_LOG/Src/mgr_log_rtc.c \
$(KINEIS_DIR)/Extdep/Mcu/Src/mcu_misc.c \
$(KINEIS_DIR)/Extdep/Mcu/Src/mcu_flash.c \
$(KINEIS_DIR)/Extdep/Mcu/Src/mcu_aes.c \
$(KINEIS_DIR)/Extdep/Mcu/Src/aes.c \
$(KINEIS_DIR)/Extdep/Mcu/Src/mcu_nvm.c \
$(KINEIS_DIR)/Extdep/Mcu/Src/mcu_tim.c \
$(KINEIS_DIR)/App/Mcu/Src/mcu_at_console.c \
$(KINEIS_DIR)/App/Libs/TRACKER/Src/tracker_app.c \
$(KINEIS_DIR)/App/Managers/MGR_AT_CMD/Src/mgr_at_cmd.c \
$(KINEIS_DIR)/App/Managers/MGR_AT_CMD/Src/mgr_at_cmd_common.c \
$(KINEIS_DIR)/App/Managers/MGR_AT_CMD/Src/mgr_at_cmd_list.c \
$(KINEIS_DIR)/App/Managers/MGR_AT_CMD/Src/mgr_at_cmd_list_user_data.c \
$(KINEIS_DIR)/App/Managers/MGR_AT_CMD/Src/mgr_at_cmd_list_general.c \
$(KINEIS_DIR)/App/Managers/MGR_AT_CMD/Src/mgr_at_cmd_list_mac.c \
$(KINEIS_DIR)/App/Managers/MGR_AT_CMD/Src/mgr_at_cmd_list_certif.c \
$(KINEIS_DIR)/App/Managers/MGR_AT_CMD/Src/mgr_at_cmd_list_previpass.c \
$(KINEIS_DIR)/App/Managers/MGR_SPI_CMD/Src/mgr_at_cmd_list_trackerapp.c \
$(KINEIS_DIR)/App/Kineis_os/KNS_Q/Src/kns_q.c \
$(KINEIS_DIR)/App/Kineis_os/KNS_OS/Src/kns_os.c \
$(KINEIS_DIR)/App/kns_app.c \
$(KINEIS_DIR)/App/Libs/STRUTIL/Src/strutil_lib.c \
$(KINEIS_DIR)/App/Libs/USERDATA/Src/user_data.c \
$(KINEIS_DIR)/Lpm/Src/mgr_lpm.c \
$(KINEIS_DIR)/Lpm/Src/lpm.c \
$(KINEIS_DIR)/Lpm/Src/lpm_cli_kstk.c

C_SOURCES += #$(libknsrf_wl_SOURCES)

C_SOURCES += $(LIB_INFO_SOURCES)

# ASM sources
ASM_SOURCES =  \
startup_stm32wl55xx_cm4.s


#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m4

# fpu
# NONE for Cortex-M0/M0+/M3

# float-abi


# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS =

# C defines
C_DEFS +=  \
-DCORE_CM4 \
-DUSE_HAL_DRIVER \
-DSTM32WL55xx \
-DUSE_LOCAL_PRINTF \
-DUSE_USERDATA_TX \
-DLPM_$(LPM)_ENABLED \
-D$(KRD_BOARD)

ifeq ($(USE_BAREMETAL), 1)
C_DEFS +=  \
-DUSE_BAREMETAL
endif

C_DEFS += #$(libknsrf_wl_C_DEFS)

ifeq ($(DEBUG), 1)
C_DEFS +=  \
-DDEBUG
endif

ifeq ($(VERBOSE), 1)
C_DEFS +=  \
-DVERBOSE
endif

ifeq ($(APP),STDLN)
C_DEFS +=  \
-DUSE_STDALONE_APP
ifeq ($(MAC_PRFL), BASIC)
C_DEFS +=  \
-DUSE_MAC_PRFL_BASIC
endif
ifeq ($(MAC_PRFL), BLIND)
C_DEFS +=  \
-DUSE_MAC_PRFL_BLIND
endif
endif

ifeq ($(APP),GUI)
C_DEFS +=  \
-DUSE_GUI_APP
endif

ifeq ($(APP),TRACKER)
C_DEFS +=  \
-DUSE_TRACKER_APP
ifeq ($(MAC_PRFL), BASIC)
C_DEFS +=  \
-DUSE_MAC_PRFL_BASIC
endif
ifeq ($(MAC_PRFL), BLIND)
C_DEFS +=  \
-DUSE_MAC_PRFL_BLIND
endif
endif

ifeq ($(COMM),UART)
C_DEFS +=  \
-DUSE_UART_DRIVER
endif
ifeq ($(COMM),SPI)
C_DEFS +=  \
-DUSE_SPI_DRIVER

C_SOURCES += Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_spi.c \
			Core/Src/spi.c \
			$(KINEIS_DIR)/App/Mcu/Src/mcu_spi_driver.c \
			$(KINEIS_DIR)/App/Managers/MGR_SPI_CMD/Src/mgr_spi_cmd.c \
			$(KINEIS_DIR)/App/Managers/MGR_SPI_CMD/Src/mgr_spi_cmd_common.c \
			$(KINEIS_DIR)/App/Managers/MGR_SPI_CMD/Src/mgr_spi_cmd_list.c \
			$(KINEIS_DIR)/App/Managers/MGR_SPI_CMD/Src/mgr_spi_cmd_list_user_data.c \
			$(KINEIS_DIR)/App/Managers/MGR_SPI_CMD/Src/mgr_spi_cmd_list_general.c \
			$(KINEIS_DIR)/App/Managers/MGR_SPI_CMD/Src/mgr_spi_cmd_list_mac.c \
			$(KINEIS_DIR)/App/Managers/MGR_SPI_CMD/Src/mgr_spi_cmd_list_certif.c \
			$(KINEIS_DIR)/App/Managers/MGR_SPI_CMD/Src/mgr_spi_cmd_list_previpass.c 
endif


# Build version

ifeq ($(DEBUG), 1)
BUILD_VERSION := $(BUILD_VERSION)D
endif
ifeq ($(VERBOSE), 1)
BUILD_VERSION := $(BUILD_VERSION)V
endif
ifeq ($(USE_RX_STACK), 1)
BUILD_VERSION := $(BUILD_VERSION)Trx
else
BUILD_VERSION := $(BUILD_VERSION)Tx
endif
ifeq ($(USE_HDA4), 1)
BUILD_VERSION := $(BUILD_VERSION)Hda4
endif

# NONE, SLEEP, STOP, STANDBY, SHUTDOWN
ifeq ($(LPM), SLEEP)
BUILD_VERSION := $(BUILD_VERSION)Slp
endif
ifeq ($(LPM), STOP)
BUILD_VERSION := $(BUILD_VERSION)Stp
endif
ifeq ($(LPM), STANDBY)
BUILD_VERSION := $(BUILD_VERSION)Stdb
endif
ifeq ($(LPM), SHUTDOWN)
BUILD_VERSION := $(BUILD_VERSION)Shtdwn
endif
ifeq ($(APP),GUI)
BUILD_VERSION := $(BUILD_VERSION)_gui
endif
ifeq ($(APP),STDLN)
BUILD_VERSION := $(BUILD_VERSION)_stdln
endif
ifeq ($(APP),TRACKER)
BUILD_VERSION := $(BUILD_VERSION)_tracker
endif
ifeq ($(MAC_PRFL), BASIC)
BUILD_VERSION := $(BUILD_VERSION)_basic
endif
ifeq ($(MAC_PRFL), BLIND)
BUILD_VERSION := $(BUILD_VERSION)_blind
endif
ifeq ($(KRD_BOARD),KRD_FW_MP)
BUILD_VERSION := $(BUILD_VERSION)_Mp
endif
ifeq ($(KRD_BOARD),KRD_FW_LP)
BUILD_VERSION := $(BUILD_VERSION)_Lp
endif

#######################################
# includes
#######################################
# AS includes
AS_INCLUDES =

# C includes
C_INCLUDES =  \
-I. \
-ICore/Inc \
-IDrivers/STM32WLxx_HAL_Driver/Inc \
-IDrivers/STM32WLxx_HAL_Driver/Inc/Legacy \
-IDrivers/CMSIS/Device/ST/STM32WLxx/Include \
-IDrivers/CMSIS/Include \
-Ikineis_sw \
-I$(KINEIS_DIR)/Lib \
-I$(KINEIS_DIR)/Extdep/Conf \
-I$(KINEIS_DIR)/Extdep/Mcu/Inc \
-I$(KINEIS_DIR)/Extdep/MGR_LOG/Inc \
-I$(KINEIS_DIR)/App/Managers/MGR_AT_CMD/Inc \
-I$(KINEIS_DIR)/App/Managers/MGR_SPI_CMD/Inc \
-I$(KINEIS_DIR)/Appconf \
-I$(KINEIS_DIR)/App/Libs/TRACKER/Inc \
-I$(KINEIS_DIR)/App/Kineis_os/KNS_Q/Inc \
-I$(KINEIS_DIR)/App/Kineis_os/KNS_OS/Inc \
-I$(KINEIS_DIR)/App/. \
-I$(KINEIS_DIR)/App/Mcu/Inc \
-I$(KINEIS_DIR)/App/Libs/STRUTIL/Inc \
-I$(KINEIS_DIR)/App/Libs/USERDATA/Inc \
-I$(KINEIS_DIR)/Lpm/Inc 

C_INCLUDES += #$(libknsrf_wl_INCLUDES)


ifeq ($(COMM),UART)
C_DEFS +=  \
-DUSE_UART_DRIVER
endif

ifeq ($(COMM),SPI)
C_DEFS +=  \
-DUSE_SPI_DRIVER
endif

# compile gcc flags
ASFLAGS += $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -Wextra -Werror -Wno-unused-but-set-variable -Wno-enum-conversion -Wno-unused-parameter -Wimplicit-fallthrough=1 -Wtype-limits -fdata-sections -Wwrite-strings -ffunction-sections -fstack-usage

CFLAGS += $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -Wextra -Werror -Wno-unused-but-set-variable -Wno-enum-conversion -Wno-unused-parameter -Wimplicit-fallthrough=1 -Wtype-limits -fdata-sections -Wwrite-strings -ffunction-sections -fstack-usage

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif


# Generate dependency information
CFLAGS += -MD -MP -MF"$(@:%.o=%.d)"

#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = STM32WL55XX_FLASH_CM4.ld

# libraries
LIBS = -lc -lm -lnosys
LIBDIR =
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections -static
ifneq ($(DEBUG), 1)
LDFLAGS += -s
endif

#######################################
# rules
#######################################
# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin
	@echo '==== Build with libkineis.a and libknsrf_wl.a completed ===='
	@echo '==== $(LIB_VERSIONS) ===='


# generate doc (doxygen framework), configure some dynamic parameters from Makefile
doc: Doxyfile $(DOC_DIR)
	(cat $< ; \
echo $(CFLAGS) | xargs -n1 echo | grep "^-D" | sed 's/-D//' | xargs echo PREDEFINED= ; \
echo PROJECT_NUMBER= $(KINEIS_VERSION); \
echo OUTPUT_DIRECTORY=$(DOC_DIR) ; \
echo INPUT= Core/Src/main.c $(KINEIS_DIR)/README.md $(subst -I,,$(C_INCLUDES)) ; \
echo EXCLUDE=$(DOC_DIR) ; \
echo EXCLUDE_PATTERNS=*/UnitTest*/* */IntegrationTest*/* */extra_inc/* */extra_src/* */FreeRTOS/*; \
echo EXCLUDE_PATTERNS+=*/Tools/*; \
echo EXCLUDE_PATTERNS+=*/Kineis/Indep/*; \
echo EXCLUDE_PATTERNS+=*/kineis_sw/*; \
echo EXCLUDE_PATTERNS+=*/Drivers/STM32*/* */Drivers/CMSIS*/* */Core/*; \
echo USE_MDFILE_AS_MAINPAGE= $(KINEIS_DIR)/README.md; \
echo WARN_LOGFILE=$(DOXY_WARN_LOGFILE)) \
| doxygen -
	-(git log -n 1 > $(DOC_DIR)/doc_version.txt)
	@echo "==== Doc generated for firmware package ===="

#######################################
# build the application
#######################################
# list of objects from C files
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
BUILD_INFO_OBJ = $(addprefix $(BUILD_DIR)/,$(notdir $(BUILD_INFO_FILE:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of objects from ASM files
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

# generate SW version of the firmware from GIT commit ID
$(BUILD_INFO_FILE): $(OBJECTS)
	@echo "-- Build file build_info.c --"
	@echo '#include "build_info.h"'                            > $(BUILD_INFO_FILE)
	@echo ''                                                   >> $(BUILD_INFO_FILE)
	@echo -n 'const char uc_fw_vers_commit_id[] = "'           >> $(BUILD_INFO_FILE)
	@echo -n '$(current_repo_commit)'                          >> $(BUILD_INFO_FILE)
	@echo "!$(current_repo_status)!"
	@if [ -n "$(current_repo_status)" ]; then                                         \
		echo -n '*'                                        >> $(BUILD_INFO_FILE); \
	fi
	@if [ -n "$(BUILD_VERSION)" ]; then                                               \
		echo -n '_$(BUILD_VERSION)'                        >> $(BUILD_INFO_FILE); \
	fi
	@if [ -n "$(LIB_VERSIONS)" ]; then                                               \
		echo -n ',$(LIB_VERSIONS)'                        >> $(BUILD_INFO_FILE); \
	fi
	@echo -n ',$(BUILD_DATE)'                                  >> $(BUILD_INFO_FILE)
	@echo '";'                                                 >> $(BUILD_INFO_FILE)


$(BUILD_DIR)/%.o: %.c $(current_makefile) | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s $(current_makefile) | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) $(BUILD_INFO_OBJ) $(current_makefile)
	@echo "-- Build firmware --"
	$(CC) $(OBJECTS) $(BUILD_INFO_OBJ) -L$(KINEIS_DIR)/Lib/. -Wl,--whole-archive -lkineis -lknsrf_wl -Wl,--no-whole-archive $(LDFLAGS) -o $@
	$(SZ) -t $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@

$(KINEIS_DIR) $(BUILD_DIR) $(DOC_DIR):
	mkdir -p $@

#######################################
# clean up
#######################################
clean: #libknsrf_wl_clean
	-rm -fR $(BUILD_INFO_FILE)
	-rm -fR $(BUILD_DIR)

doc_clean:
	-rm -fR $(DOC_DIR)
	-rm -f  $(DOXY_WARN_LOGFILE)

#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

.PHONY: doc doc_clean

#######################################
# force empty target
#######################################
.DEFAULT_GOAL := all

# *** EOF ***
