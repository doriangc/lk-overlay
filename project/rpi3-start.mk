LOCAL_DIR := $(GET_LOCAL_DIR)

TARGET := rpi3-vpu

MODULES += \
	app/shell \
	app/stringtests \
	app/tests \
	lib/cksum \
	lib/debugcommands \
	lib/gfx \
	# platform/bcm28xx/hvs-dance \
	platform/bcm28xx/otp \
	lib/tga

# MODULES += lib/gfxconsole
# MODULES += lib/fasterconsole

GLOBAL_DEFINES += PRIMARY_HVS_CHANNEL=1
# MODULES += platform/bcm28xx/vec
MODULES += platform/bcm28xx/vc4-hdmi
