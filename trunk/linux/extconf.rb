#!/usr/bin/env ruby

WINDOWS_HINTS = %w(-win32 win32- mswin mingw32)

if defined? RUBY_PLATFORM and WINDOWS_HINTS.any? { |hint| RUBY_PLATFORM.contain? hint } then
  puts 'This gem is not meant to be installed on Windows. Instead, please use:'
  puts 'gem install gosu --platform=i386-mingw32'
  exit 1
end

puts 'The Gosu gem requires some libraries to be installed system-wide.'
puts 'See the following site for a list:'
puts 'http://code.google.com/p/gosu/wiki/GettingStartedOnLinux'
puts

# FIXME should reversely filter out files ending in Win, Mac, Touch, AL etc.
SOURCE_FILES = %w(
  Audio/AudioSDL.cpp
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
  Graphics/TextUnix.cpp
  Graphics/Texture.cpp
  Graphics/Transform.cpp
  InputX.cpp
  Inspection.cpp
  IO.cpp
  Math.cpp
  RubyGosu_wrap.cxx
  TextInputX.cpp
  TimingUnix.cpp
  Utility.cpp
  WindowX.cpp
)

require 'mkmf'

# Copy all relevant C++ files into the current directory
# FIXME Could be done by gem task instead.
SOURCE_FILES.each { |file| `cp ../GosuImpl/#{file} #{File.basename(file)}` }

# Symlink our pretty gosu.so into ../lib
# FIXME gosu.rb should just look in the right place.
`ln -s ../linux/gosu.so ../lib/gosu.so`

sdl_config = with_config("sdl-config", "sdl-config")
pango_config = "pkg-config pangoft2"

$INCFLAGS << " -I../ -I../GosuImpl `#{sdl_config} --cflags` `#{pango_config} --cflags`"
$LDFLAGS << " `#{sdl_config} --libs` `#{pango_config} --libs` -lX11"
have_header('SDL_ttf.h') if have_library('SDL_ttf', 'TTF_RenderUTF8_Blended')
have_header('gl.h') if have_library('GL', 'glMatrixMode')
have_header('png.h') if have_library('png', 'png_sig_cmp')

create_makefile("gosu")
