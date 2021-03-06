# Settings for eta ecm3531 platform
ifeq ($(TARGET), ecm3531)
  TARGET_ARCH := cortex-m3
  TARGET_TOOLCHAIN_PREFIX := arm-none-eabi-
  ETA_SDK := /home/hari/TensaiSDK-v0.0.17/soc/
  GCC_ARM := /home/hari/Downloads/gcc-arm-none-eabi-7-2018-q2-update/

#Pick the appropriate lds file depending whether you are running frof SRAM of flash
  ETA_LDS_FILE := ecm3531.lds
#  ETA_LDS_FILE := ecm3531_flash.lds

  ifeq ($(wildcard $(ETA_SDK)),)
    $(error Path to ETA SDK is not set (ETA_SDK))
  endif

  ifeq ($(wildcard $(GCC_ARM)),)
    $(error Path to gcc arm compiler is not set (GCC_ARM))
  endif

  PLATFORM_FLAGS = \
    -DARM_MATH_CM3 \
    -DFIRMWARE_BUILD \
    -DGEMMLOWP_ALLOW_SLOW_SCALAR_FALLBACK \
    -DTF_LITE_STATIC_MEMORY \
    -DTF_LITE_MCU_DEBUG_LOG \
    -fno-rtti \
    -fmessage-length=0 \
    -fno-exceptions \
    -fno-unwind-tables \
    -fno-builtin \
    -ffunction-sections \
    -fdata-sections \
    -funsigned-char \
    -MMD \
    -mcpu=cortex-m3 \
    -mthumb \
    -mlittle-endian \
    -mno-unaligned-access \
    -std=gnu++11 \
    -Wvla \
    -Wall \
    -Wextra \
    -Wsign-compare \
    -Wdouble-promotion \
    -Wshadow \
    -Wunused-variable \
    -Wmissing-field-initializers \
    -Wno-unused-parameter \
    -Wno-write-strings \
    -fno-delete-null-pointer-checks \
    -fno-threadsafe-statics \
    -fomit-frame-pointer \
    -fpermissive \
    -fno-use-cxa-atexit \
    -nostdlib \
    -g \
    -Os
  CXXFLAGS += $(PLATFORM_FLAGS)
  CCFLAGS += $(PLATFORM_FLAGS)
# Adding the --specs=nano.specs flag causes the linker to use libc_nano.a
# instead of libc.a.  This gets rid of lots of errors with various pieces
# of the exception unwinding code not being found.  Not clear why it is
# trying to link in this code to begin with, though.
  LDFLAGS += \
    -mthumb -mcpu=cortex-m3 \
    -nostartfiles -static \
    -Wl,--gc-sections -Wl,--entry,ResetISR \
    -Wl,--start-group -lm -lc -lgcc -Wl,--end-group \
    -fno-exceptions \
    -nostdlib --specs=nano.specs -t -lstdc++ -lc -lnosys -lm \
    -Wl,-T,$(MAKEFILE_DIR)/targets/ecm3531/$(ETA_LDS_FILE) \
    -Wl,-Map=$(MAKEFILE_DIR)/targets/ecm3531/ecm3531.map,--cref
  BUILD_TYPE := micro
  MICROLITE_LIBS := \
    $(GCC_ARM)/lib/gcc/arm-none-eabi/7.3.1/thumb/v7e-m/fpv4-sp/softfp/crtbegin.o \
    -lm
  ECM3531_INCLUDES := \
    -I$(GCC_ARM)/arm-none-eabi/include/ \
    -I$(ETA_SDK)/ecm3531/boards/eta_evb/projects/m3/common/inc/ \
    -I$(ETA_SDK)/ecm3531/m3/reg/inc/ \
    -I$(ETA_SDK)/ecm3531/m3/csp/inc/ \
    -I$(ETA_SDK)/ecm3531/common/csp/inc/ \
    -I$(ETA_SDK)/common/inc/  \
    -I$(ETA_SDK)/../utils/inc/  \
    -I$(ETA_SDK)/ecm3531/boards/eta_evb/eta_bsp/inc

  INCLUDES += $(ECM3531_INCLUDES)
  GENERATED_PROJECT_INCLUDES += $(ECM3531_INCLUDES)

  # _main.c contains application and target specific initialization, like
  # setting clock speed, default uart setups, etc. and an implementation
#of the DebugLog interfaces.
  MICROLITE_CC_SRCS += \
    $(MAKEFILE_DIR)/targets/ecm3531/startup.c \
    $(MAKEFILE_DIR)/targets/ecm3531/_main.c \
    $(wildcard $(ETA_SDK)/ecm3531/boards/eta_evb/projects/m3/common/src/*.c) \
    $(wildcard $(ETA_SDK)/ecm3531/m3/csp/src/*.c) \
    $(wildcard $(ETA_SDK)/ecm3531/m3/csp/src/*.s)

  # The linker script isn't a header, but it needs to get copied to the gen/
  # directory for generated projects.  This is similar to the behavior needed
  # for headers.
  MICROLITE_CC_HDRS += \
    $(MAKEFILE_DIR)/targets/ecm3531/$(ETA_LDS_FILE)

  TEST_SCRIPT := tensorflow/lite/micro/testing/test_ecm3531_binary.sh
  # These are tests that don't currently work on the blue pill.
  EXCLUDED_TESTS := \
    tensorflow/lite/micro/micro_interpreter_test.cc \
    tensorflow/lite/micro/simple_tensor_allocator_test.cc
  MICROLITE_TEST_SRCS := $(filter-out $(EXCLUDED_TESTS), $(MICROLITE_TEST_SRCS))

# These are microcontroller-specific rules for converting the ELF output
# of the linker into a binary image that can be loaded directly.
  OBJCOPY := $(TARGET_TOOLCHAIN_PREFIX)objcopy

  $(BINDIR)/%.bin: $(BINDIR)/%
	  @mkdir -p $(dir $@)
	  $(OBJCOPY) $< $@ -O binary

endif
