LOCAL_DIR := $(GET_LOCAL_DIR)

TARGET := rpi3-vpu

MODULES += \
	platform/bcm28xx/otp \
	#platform/bcm28xx/rpi-ddr2/autoram \
	#app/shell \
	#lib/debugcommands \
	#lib/fs/ext2 \
	#app/mountroot \

MODULES += platform/bcm28xx/dpi
GLOBAL_DEFINES += PRIMARY_HVS_CHANNEL=0
GLOBAL_DEFINES += UART_NO_MUX

GLOBAL_DEFINES += BOOTCODE=1
#GLOBAL_DEFINES += NOVM_MAX_ARENAS=2 NOVM_DEFAULT_ARENA=0
GLOBAL_DEFINES += WITH_NO_FP=1 BACKGROUND=0xffffff
#GLOBAL_DEFINES += CUSTOM_DEFAULT_STACK_SIZE=1024
BOOTCODE := 1
DEBUG := 0

