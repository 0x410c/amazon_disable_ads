LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
  dirtyc0w.c

LOCAL_MODULE := dirtycow
LOCAL_LDFLAGS   += -llog
LOCAL_CFLAGS    += -DDEBUG

include $(BUILD_EXECUTABLE)
