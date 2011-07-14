# Makefile for use with MinGW

SRCS = GosuImpl/Sockets/CommSocket.cpp GosuImpl/DirectoriesWin.cpp GosuImpl/FileWin.cpp GosuImpl/InputWin.cpp GosuImpl/Inspection.cpp GosuImpl/IO.cpp GosuImpl/Sockets/ListenerSocket.cpp GosuImpl/Math.cpp GosuImpl/Sockets/MessageSocket.cpp GosuImpl/Sockets/Socket.cpp GosuImpl/TextInputWin.cpp GosuImpl/TimingWin.cpp GosuImpl/Utility.cpp GosuImpl/WindowWin.cpp GosuImpl/WinMain.cpp GosuImpl/WinUtility.cpp GosuImpl/Graphics/Bitmap.cpp GosuImpl/Graphics/BitmapColorKey.cpp GosuImpl/Graphics/BitmapFreeImage.cpp GosuImpl/Graphics/BitmapUtils.cpp GosuImpl/Graphics/BlockAllocator.cpp GosuImpl/Graphics/Color.cpp GosuImpl/Graphics/Font.cpp GosuImpl/Graphics/Graphics.cpp GosuImpl/Graphics/Image.cpp GosuImpl/Graphics/LargeImageData.cpp GosuImpl/Graphics/TexChunk.cpp GosuImpl/Graphics/Text.cpp GosuImpl/Graphics/TextTTFWin.cpp GosuImpl/Graphics/Texture.cpp GosuImpl/Graphics/TextWin.cpp GosuImpl/Graphics/Transform.cpp GosuImpl/Audio/AudioSDL.cpp

OBJS = $(SRCS:.cpp=.o)

DXINSTALL = $(shell 'cmd.exe' /c 'reg query HKLM\Software\Microsoft\DirectX /s' | grep InstallPath | sed 's/.*REG_SZ\s//' | head -1)
DXINCLUDE = "$(DXINSTALL)\Include"

CXXFLAGS = -g -I. -I$(DXINCLUDE) -DUNICODE -D_UNICODE -DMINGW

all: directx_sdk $(OBJS)
	ar rcs lib/libgosu.a $(OBJS)

directx_sdk:
ifeq ($(DXINSTALL),)
	@echo Cannot find installed DirectX SDK.
	@false
endif

clean:
	rm -f $(OBJS) lib/libgosu.a

install: all
	install lib/libgosu.a /usr/lib/
	mkdir -p /usr/include/Gosu/
	install Gosu/* /usr/include/Gosu/

