#

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := csetup 
LOCAL_SRC_FILES := \
	libpng/pngwutil.c libpng/pngwtran.c libpng/pngwrite.c libpng/pngwio.c libpng/pngtrans.c \
	libpng/pngset.c libpng/pngrutil.c libpng/pngrtran.c libpng/pngrio.c libpng/pngread.c \
	libpng/pngpread.c libpng/pngmem.c libpng/pngget.c libpng/pngerror.c libpng/png.c \
	csetup.cpp

LOCAL_CFLAGS :=-I./libpng/

LOCAL_LDLIBS := -static -lm -lc -lstdc++ 
include $(BUILD_EXECUTABLE)
