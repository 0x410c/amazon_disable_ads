LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
  dirtyc0w.c

LOCAL_CFLAGS += 
LOCAL_MODULE := getroot
LOCAL_LDFLAGS += -static

include $(BUILD_EXECUTABLE)
