LOCAL_PATH := $(call my-dir)

###########################
#
# Gosu shared library
#
###########################

include $(CLEAR_VARS)

LOCAL_MODULE := gosu

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include \
	$(wildcard $(LOCAL_PATH)/dependencies/SDL/include) \
	$(wildcard $(LOCAL_PATH)/dependencies/stb) \
	$(wildcard $(LOCAL_PATH)/dependencies/utf8proc) \
	$(wildcard $(LOCAL_PATH)/src/*.hpp)
#	$(wildcard $(LOCAL_PATH)../al_soft/include) \
#	$(wildcard $(LOCAL_PATH)/dependencies/SDL_sound) \
#	$(wildcard $(LOCAL_PATH)/dependencies/SDL_sound/libmodplug) \

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)

LOCAL_SRC_FILES := \
	$(wildcard $(LOCAL_PATH)/src/Bitmap.cpp) \
	$(wildcard $(LOCAL_PATH)/src/BitmapIO.cpp) \
	$(wildcard $(LOCAL_PATH)/src/BlockAllocator.cpp) \
	$(wildcard $(LOCAL_PATH)/src/Color.cpp) \
	$(wildcard $(LOCAL_PATH)/src/DirectoriesUnix.cpp) \
	$(wildcard $(LOCAL_PATH)/src/FileAndroid.cpp) \
	$(wildcard $(LOCAL_PATH)/src/Font.cpp) \
	$(wildcard $(LOCAL_PATH)/src/Graphics.cpp) \
	$(wildcard $(LOCAL_PATH)/src/IO.cpp) \
	$(wildcard $(LOCAL_PATH)/src/Image.cpp) \
	$(wildcard $(LOCAL_PATH)/src/Input.cpp) \
	$(wildcard $(LOCAL_PATH)/src/Inspection.cpp) \
	$(wildcard $(LOCAL_PATH)/src/LargeImageData.cpp) \
	$(wildcard $(LOCAL_PATH)/src/Macro.cpp) \
	$(wildcard $(LOCAL_PATH)/src/MarkupParser.cpp) \
	$(wildcard $(LOCAL_PATH)/src/Math.cpp) \
	$(wildcard $(LOCAL_PATH)/src/Resolution.cpp) \
	$(wildcard $(LOCAL_PATH)/src/TexChunk.cpp) \
	$(wildcard $(LOCAL_PATH)/src/Text.cpp) \
	$(wildcard $(LOCAL_PATH)/src/TextBuilder.cpp) \
	$(wildcard $(LOCAL_PATH)/src/TextInput.cpp) \
	$(wildcard $(LOCAL_PATH)/src/Texture.cpp) \
	$(wildcard $(LOCAL_PATH)/src/TimingUnix.cpp) \
	$(wildcard $(LOCAL_PATH)/src/Transform.cpp) \
	$(wildcard $(LOCAL_PATH)/src/TrueTypeFont.cpp) \
	$(wildcard $(LOCAL_PATH)/src/TrueTypeFontAndroid.cpp) \
	$(wildcard $(LOCAL_PATH)/src/Utility.cpp) \
	$(wildcard $(LOCAL_PATH)/src/Version.cpp) \
	$(wildcard $(LOCAL_PATH)/src/Window.cpp) \
	$(wildcard $(LOCAL_PATH)/dependencies/utf8proc/*.c) \

# Disable Audio for now
#	$(wildcard $(LOCAL_PATH)/src/TrueTypeFontUnix.cpp) \
#	$(wildcard $(LOCAL_PATH)/src/OffScreenTarget.cpp) \
#	$(wildcard $(LOCAL_PATH)/src/Audio.cpp) \
#	$(wildcard $(LOCAL_PATH)/src/AudioFileAudioToolbox.cpp) \
#	$(wildcard $(LOCAL_PATH)/src/AudioFileSDLSound.cpp) \
#	$(wildcard $(LOCAL_PATH)/src/AudioImpl.cpp) \
#	$(wildcard $(LOCAL_PATH)/src/Channel.cpp) \
#	$(wildcard $(LOCAL_PATH)/dependencies/SDL_sound/*.c) \
#	$(wildcard $(LOCAL_PATH)/dependencies/SDL_sound/libmodplug/*.c) \

LOCAL_SHARED_LIBRARIES := SDL2 SDL2_ttf

LOCAL_CFLAGS += -DANDROID

LOCAL_CPPFLAGS += -fexceptions -std=c++17 -fstack-protector
 

LOCAL_LDLIBS := -llog -landroid -lEGL -lGLESv1_CM

ifeq ($(NDK_DEBUG),1)
    cmd-strip :=
endif

include $(BUILD_SHARED_LIBRARY)

