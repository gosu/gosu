LOCAL_PATH := $(call my-dir)

###########################
#
# Gosu shared library
#
###########################

include $(CLEAR_VARS)

LOCAL_MODULE := gosu

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include \
	$(wildcard $(LOCAL_PATH)/src/*.hpp)

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)

LOCAL_SRC_FILES := \
	$(wildcard $(LOCAL_PATH)/src/*.cpp)
#	$(LOCAL_PATH)/src/Audio.cpp \
#	$(LOCAL_PATH)/src/AudioFile.cpp \
#	$(LOCAL_PATH)/src/AudioFileSDLSound.cpp \
#	$(LOCAL_PATH)/src/AudioImpl.cpp \
#	$(LOCAL_PATH)/src/AudioImpl.hpp \
#	$(LOCAL_PATH)/src/Bitmap.cpp \
#	$(LOCAL_PATH)/src/BitmapIO.cpp \
#	$(LOCAL_PATH)/src/BlockAllocator.cpp \
#	$(LOCAL_PATH)/src/Channel.cpp \
#	$(LOCAL_PATH)/src/ClipRectStack.hpp \
#	$(LOCAL_PATH)/src/Color.cpp \
#	$(LOCAL_PATH)/src/DrawOp.hpp \
#	$(LOCAL_PATH)/src/DrawOpQueue.hpp \
#	$(LOCAL_PATH)/src/EmptyImageData.hpp \
#	$(LOCAL_PATH)/src/Font.cpp \
#	$(LOCAL_PATH)/src/Graphics.cpp \
#	$(LOCAL_PATH)/src/Iconv.cpp \
#	$(LOCAL_PATH)/src/Image.cpp \
#	$(LOCAL_PATH)/src/Input.cpp \
#	$(LOCAL_PATH)/src/Inspection.cpp \
#	$(LOCAL_PATH)/src/IO.cpp \
#	$(LOCAL_PATH)/src/LargeImageData.cpp \
#	$(LOCAL_PATH)/src/LargeImageData.hpp \
#	$(LOCAL_PATH)/src/Log.hpp \
#	$(LOCAL_PATH)/src/Macro.cpp \
#	$(LOCAL_PATH)/src/Macro.hpp \
#	$(LOCAL_PATH)/src/MarkupParser.cpp \
#	$(LOCAL_PATH)/src/MarkupParser.hpp \
#	$(LOCAL_PATH)/src/Math.cpp \
#	$(LOCAL_PATH)/src/OffScreenTarget.cpp \

LOCAL_SHARED_LIBRARIES :=

LOCAL_CFLAGS += -DANDROID


LOCAL_CFLAGS +=
 

LOCAL_LDLIBS :=

ifeq ($(NDK_DEBUG),1)
    cmd-strip :=
endif

include $(BUILD_SHARED_LIBRARY)

