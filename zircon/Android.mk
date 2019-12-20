LOCAL_PATH := $(my-dir)

SHARED_CFLAGS := \
	$(KERNELFLINGER_CFLAGS)

# Build libavb for the target (for e.g. fs_mgr usage).
include $(CLEAR_VARS)
LOCAL_MODULE := libzircon-$(TARGET_BUILD_VARIANT)
LOCAL_MODULE_HOST_OS := linux
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)
#LOCAL_CLANG := true
LOCAL_CFLAGS := $(SHARED_CFLAGS) -Wno-error

LOCAL_STATIC_LIBRARIES := \
	$(KERNELFLINGER_STATIC_LIBRARIES) \
	libkernelflinger-$(TARGET_BUILD_VARIANT)

LOCAL_SRC_FILES := \
	src/cmdline.c \
	src/misc.c \
	src/pci.c \
	src/zircon.c

LOCAL_C_INCLUDES += $(LOCAL_PATH) \
	$(addprefix $(LOCAL_PATH)/,include) \
	$(addprefix $(LOCAL_PATH)/,src) \
	$(addprefix $(LOCAL_PATH)/../,include) \

include $(BUILD_EFI_STATIC_LIBRARY)
