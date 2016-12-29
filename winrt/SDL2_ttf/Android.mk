LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := SDL2_ttf

FREETYPE_LIBRARY_PATH := external/freetype-2.4.12

LOCAL_C_INCLUDES := $(LOCAL_PATH)

LOCAL_SRC_FILES := SDL_ttf.c \

ifneq ($(FREETYPE_LIBRARY_PATH),)
    LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(FREETYPE_LIBRARY_PATH)/include
    LOCAL_CFLAGS += -DFT2_BUILD_LIBRARY
    LOCAL_SRC_FILES += \
        $(FREETYPE_LIBRARY_PATH)/src/autofit/autofit.c \
        $(FREETYPE_LIBRARY_PATH)/src/base/ftbase.c \
        $(FREETYPE_LIBRARY_PATH)/src/base/ftbbox.c \
        $(FREETYPE_LIBRARY_PATH)/src/base/ftbdf.c \
        $(FREETYPE_LIBRARY_PATH)/src/base/ftbitmap.c \
        $(FREETYPE_LIBRARY_PATH)/src/base/ftcid.c \
        $(FREETYPE_LIBRARY_PATH)/src/base/ftdebug.c \
        $(FREETYPE_LIBRARY_PATH)/src/base/ftfstype.c \
        $(FREETYPE_LIBRARY_PATH)/src/base/ftgasp.c \
        $(FREETYPE_LIBRARY_PATH)/src/base/ftglyph.c \
        $(FREETYPE_LIBRARY_PATH)/src/base/ftgxval.c \
        $(FREETYPE_LIBRARY_PATH)/src/base/ftinit.c \
        $(FREETYPE_LIBRARY_PATH)/src/base/ftlcdfil.c \
        $(FREETYPE_LIBRARY_PATH)/src/base/ftmm.c \
        $(FREETYPE_LIBRARY_PATH)/src/base/ftotval.c \
        $(FREETYPE_LIBRARY_PATH)/src/base/ftpatent.c \
        $(FREETYPE_LIBRARY_PATH)/src/base/ftpfr.c \
        $(FREETYPE_LIBRARY_PATH)/src/base/ftstroke.c \
        $(FREETYPE_LIBRARY_PATH)/src/base/ftsynth.c \
        $(FREETYPE_LIBRARY_PATH)/src/base/ftsystem.c \
        $(FREETYPE_LIBRARY_PATH)/src/base/fttype1.c \
        $(FREETYPE_LIBRARY_PATH)/src/base/ftwinfnt.c \
        $(FREETYPE_LIBRARY_PATH)/src/base/ftxf86.c \
        $(FREETYPE_LIBRARY_PATH)/src/bdf/bdf.c \
        $(FREETYPE_LIBRARY_PATH)/src/bzip2/ftbzip2.c \
        $(FREETYPE_LIBRARY_PATH)/src/cache/ftcache.c \
        $(FREETYPE_LIBRARY_PATH)/src/cff/cff.c \
        $(FREETYPE_LIBRARY_PATH)/src/cid/type1cid.c \
        $(FREETYPE_LIBRARY_PATH)/src/gzip/ftgzip.c \
        $(FREETYPE_LIBRARY_PATH)/src/lzw/ftlzw.c \
        $(FREETYPE_LIBRARY_PATH)/src/pcf/pcf.c \
        $(FREETYPE_LIBRARY_PATH)/src/pfr/pfr.c \
        $(FREETYPE_LIBRARY_PATH)/src/psaux/psaux.c \
        $(FREETYPE_LIBRARY_PATH)/src/pshinter/pshinter.c \
        $(FREETYPE_LIBRARY_PATH)/src/psnames/psmodule.c \
        $(FREETYPE_LIBRARY_PATH)/src/raster/raster.c \
        $(FREETYPE_LIBRARY_PATH)/src/sfnt/sfnt.c \
        $(FREETYPE_LIBRARY_PATH)/src/smooth/smooth.c \
        $(FREETYPE_LIBRARY_PATH)/src/tools/apinames.c \
        $(FREETYPE_LIBRARY_PATH)/src/truetype/truetype.c \
        $(FREETYPE_LIBRARY_PATH)/src/type1/type1.c \
        $(FREETYPE_LIBRARY_PATH)/src/type42/type42.c \
        $(FREETYPE_LIBRARY_PATH)/src/winfonts/winfnt.c
endif

LOCAL_SHARED_LIBRARIES := SDL2

LOCAL_EXPORT_C_INCLUDES += $(LOCAL_C_INCLUDES)

include $(BUILD_SHARED_LIBRARY)
