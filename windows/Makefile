# Makefile for use with MinGW

SRCS = src/Sockets/CommSocket.cpp src/DirectoriesWin.cpp src/FileWin.cpp src/InputWin.cpp src/Inspection.cpp src/IO.cpp src/Sockets/ListenerSocket.cpp src/Math.cpp src/Sockets/MessageSocket.cpp src/Sockets/Socket.cpp src/TextInputWin.cpp src/TimingWin.cpp src/Utility.cpp src/WindowWin.cpp src/WinMain.cpp src/WinUtility.cpp src/Graphics/Bitmap.cpp src/Graphics/BitmapColorKey.cpp src/Graphics/BitmapFreeImage.cpp src/Graphics/BitmapUtils.cpp src/Graphics/BlockAllocator.cpp src/Graphics/Color.cpp src/Graphics/Font.cpp src/Graphics/Graphics.cpp src/Graphics/Image.cpp src/Graphics/LargeImageData.cpp src/Graphics/TexChunk.cpp src/Graphics/Text.cpp src/Graphics/TextTTFWin.cpp src/Graphics/Texture.cpp src/Graphics/TextWin.cpp src/Graphics/Transform.cpp src/Audio/AudioSDL.cpp

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

