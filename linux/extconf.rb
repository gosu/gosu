#!/usr/bin/env ruby

WINDOWS_HINTS = %w(-win32 win32- mswin mingw32)

if defined? RUBY_PLATFORM and WINDOWS_HINTS.any? { |hint| RUBY_PLATFORM.include? hint } then
  puts 'This gem is not meant to be installed on Windows. Instead, please use:'
  puts 'gem install gosu --platform=i386-mingw32'
  exit 1
end

puts 'The Gosu gem requires some libraries to be installed system-wide.'
puts 'See the following site for a list:'
puts 'http://code.google.com/p/gosu/wiki/GettingStartedOnLinux'
puts

BASE_FILES = %w(
  DirectoriesUnix.cpp
  FileUnix.cpp
  Graphics/Bitmap.cpp
  Graphics/BitmapBMP.cpp
  Graphics/BitmapColorKey.cpp
  Graphics/BitmapPNG.cpp
  Graphics/BitmapUtils.cpp
  Graphics/BlockAllocator.cpp
  Graphics/Color.cpp
  Graphics/Font.cpp
  Graphics/Graphics.cpp
  Graphics/Image.cpp
  Graphics/LargeImageData.cpp
  Graphics/TexChunk.cpp
  Graphics/Text.cpp
  Graphics/Texture.cpp
  Graphics/Transform.cpp
  Inspection.cpp
  IO.cpp
  Math.cpp
  RubyGosu_wrap.cxx
  Utility.cpp
)

MAC_FILES = %w(
  Audio/AudioOpenAL.mm
  Graphics/BitmapApple.mm
  Graphics/TextMac.cpp
  Graphics/TextTouch.mm
  InputMac.mm
  TextInputMac.mm
  TimingApple.cpp
  WindowMac.mm
)

LINUX_FILES = %w(
  Audio/AudioSDL.cpp
  Graphics/TextUnix.cpp
  InputX.cpp
  TextInputX.cpp
  TimingUnix.cpp
  WindowX.cpp
)

require 'mkmf'

$INCFLAGS << " -I../ -I../GosuImpl"

if `uname`.chomp == 'Darwin' then
  SOURCE_FILES = BASE_FILES + MAC_FILES
  
  # The X11 paths are here because Apple curiously distributes libpng only inside X11
  $INCFLAGS << " -I/usr/local/include -I/usr/X11/include"
  $CFLAGS   << " -x objective-c++" # Never hurts!
  $LDFLAGS  << " -L/usr/local/lib -L/usr/X11/lib -lz -lpng -logg -lvorbis -lvorbisfile -liconv"
  %w(AudioToolbox IOKit OpenAL OpenGL AppKit ApplicationServices Foundation Carbon).each do |f|
    $INCFLAGS << " -framework #{f}"
    $LDFLAGS  << " -framework #{f}"
  end

  # Symlink our pretty gosu.so into ../lib
  # FIXME gosu.rb should just look in the right place.
  `ln -s ../linux/gosu.bundle ../lib/gosu.bundle`
else
  SOURCE_FILES = BASE_FILES + LINUX_FILES

  # Symlink our pretty gosu.so into ../lib
  # FIXME gosu.rb should just look in the right place.
  `ln -s ../linux/gosu.so ../lib/gosu.so`

  sdl_config = with_config("sdl-config", "sdl-config")
  pango_config = "pkg-config pangoft2"

  $INCFLAGS << " `#{sdl_config} --cflags` `#{pango_config} --cflags`"
  $LDFLAGS  << " `#{sdl_config} --libs`   `#{pango_config} --libs` -lX11"
  have_header('SDL_mixer.h') if have_library('SDL_mixer', 'Mix_OpenAudio')
  have_header('SDL_ttf.h') if have_library('SDL_ttf', 'TTF_RenderUTF8_Blended')
  have_header('gl.h') if have_library('GL', 'glMatrixMode')
  have_header('png.h') if have_library('png', 'png_sig_cmp')
end

# Copy all relevant C++ files into the current directory
# FIXME Could be done by gem task instead.
SOURCE_FILES.each { |file| `cp ../GosuImpl/#{file} #{File.basename(file).sub(/\.mm$/, '.cpp')}` }

create_makefile("gosu")
