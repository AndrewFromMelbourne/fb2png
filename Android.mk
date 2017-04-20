LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	fb2png.c

LOCAL_SHARED_LIBRARIES := \
	libpng \
	libcutils

LOCAL_MODULE:= fb2png

LOCAL_CFLAGS := -Wno-unused-parameter

include $(BUILD_EXECUTABLE)
