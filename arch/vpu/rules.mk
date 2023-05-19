LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

TOOLCHAIN_PREFIX := "/home/dorian/vc4-toolchain/prefix/bin/vc4-elf-"

$(BUILDDIR)/system-onesegment.ld: $(LOCAL_DIR)/start.ld
	@echo generating $@
	@$(MKDIR)
	echo TODO: properly template the linker script
	cp $< $@

# TODO, fix the linker flags
ARCH_LDFLAGS += -L/home/dorian/vc4-toolchain/prefix/lib/gcc/vc4-elf/6.2.1
ARCH_CFLAGS += -fstack-usage -funroll-loops -Os

MODULE_SRCS += \
	$(LOCAL_DIR)/arch.c \
	$(LOCAL_DIR)/thread.c \
	$(LOCAL_DIR)/intc.c \
	$(LOCAL_DIR)/start.S \
	$(LOCAL_DIR)/thread_asm.S \
	$(LOCAL_DIR)/interrupt.S \

MODULE_DEPS += dev/timer/vc4
GLOBAL_DEFINES += VC4_TIMER_CHANNEL=0 ARCH_HAS_MMU=0 USE_BUILTIN_ATOMICS=0

WITH_LINKER_GC ?= 1

include make/module.mk
