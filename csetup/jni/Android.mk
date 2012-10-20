#

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := csetup 
LOCAL_SRC_FILES := csetup.cpp
LOCAL_LDLIBS := -static -lm -lc -lstdc++ 
include $(BUILD_EXECUTABLE)
