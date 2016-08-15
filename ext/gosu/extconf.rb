#!/usr/bin/env ruby

if RUBY_PLATFORM =~ /mswin$|mingw32|mingw64|win32\-|\-win32/ then
  platform = (RUBY_PLATFORM =~ /^x64-/ ? 'x64-mingw32' : 'i386-mingw32')
  
  puts "This gem is not meant to be installed on Windows. Instead, please use:"
  puts "gem install gosu --platform=#{platform}"
  exit 1
end

puts 'The Gosu gem requires some libraries to be installed system-wide.'
puts 'See the following site for a list:'
if `uname`.chomp == 'Darwin' then
  puts 'https://github.com/gosu/gosu/wiki/Getting-Started-on-OS-X'
else
  puts 'https://github.com/gosu/gosu/wiki/Getting-Started-on-Linux'
end

BASE_FILES = %w(
  Bitmap/Bitmap.cpp
  Bitmap/BitmapIO.cpp
  DirectoriesUnix.cpp
  FileUnix.cpp
  Graphics/BlockAllocator.cpp
  Graphics/Color.cpp
  Graphics/Graphics.cpp
  Graphics/Image.cpp
  Graphics/LargeImageData.cpp
  Graphics/Macro.cpp
  Graphics/Resolution.cpp
  Graphics/TexChunk.cpp
  Graphics/Texture.cpp
  Graphics/Transform.cpp
  Input/Input.cpp
  Input/TextInput.cpp
  Inspection.cpp
  IO.cpp
  Math.cpp
  Text/Font.cpp
  Text/Text.cpp
  Utility.cpp
  Window.cpp
  
  stb_vorbis.c
)

MAC_FILES = %w(
  Audio/Audio.mm
  Graphics/ResolutionApple.mm
  Text/TextApple.mm
  TimingApple.cpp
  UtilityApple.mm
)

LINUX_FILES = %w(
  Audio/Audio.cpp
  Text/TextUnix.cpp
  TimingUnix.cpp
)

require 'mkmf'
require 'fileutils'

# Silence internal deprecation warnings in Gosu
$CFLAGS << " -DGOSU_DEPRECATED="

$CXXFLAGS ||= ''

$INCFLAGS << " -I../.. -I../../src"

if `uname`.chomp == 'Darwin' then
  SOURCE_FILES = BASE_FILES + MAC_FILES
  
  # To make everything work with the Objective C runtime
  $CFLAGS   << " -x objective-c -fobjc-arc -DNDEBUG"
  # Compile all C++ files as Objective C++ on OS X since mkmf does not support .mm
  # files.
  $CXXFLAGS << " -x objective-c++ -fobjc-arc -DNDEBUG"

  # Enable C++ 11, and explicitly specify libc++ as the standard library.
  # rvm will sometimes try to override this:
  # https://github.com/shawn42/gamebox/issues/96
  $CXXFLAGS << " -std=gnu++11 -stdlib=libc++"
  
  # Dependencies.
  $CXXFLAGS << " #{`sdl2-config --cflags`.chomp}"
  # Prefer statically linking SDL 2.
  $LDFLAGS  << " #{`sdl2-config --static-libs`.chomp} -framework OpenGL -framework OpenAL"
  
  # Disable building of 32-bit slices in Apple's Ruby.
  # (RbConfig::CONFIG['CXXFLAGS'] on 10.11: -arch x86_64 -arch i386 -g -Os -pipe)
  $CFLAGS.gsub! "-arch i386", ""
  $CXXFLAGS.gsub! "-arch i386", ""
  $LDFLAGS.gsub! "-arch i386", ""
  CONFIG['LDSHARED'].gsub! "-arch i386", ""
else
  SOURCE_FILES = BASE_FILES + LINUX_FILES

  if /Raspbian/ =~ `cat /etc/issue` or /BCM2708/ =~ `cat /proc/cpuinfo` then
    $INCFLAGS << " -I/opt/vc/include/GLES"
    $INCFLAGS << " -I/opt/vc/include"
    $LDFLAGS  << " -L/opt/vc/lib"
    $LDFLAGS  << " -lGLESv1_CM"
  else
    pkg_config 'gl'
  end

  pkg_config 'sdl2'
  pkg_config 'pangoft2'
  pkg_config 'vorbisfile'
  pkg_config 'openal'
  pkg_config 'sndfile'
  
  have_header 'SDL_ttf.h'   if have_library('SDL2_ttf', 'TTF_RenderUTF8_Blended')
  have_header 'AL/al.h'     if have_library('openal')
end

# And now it gets ridiculous (or I am overcomplicating things...):
# mkmf will compile all .c/.cpp files in this directory, but if they are nested
# inside folders, it will not find the resulting .o files during linking.
# So we create a shim .c/.cpp file for each file that we want to compile, ensuring
# that all .o files are built into the current directory, without any nesting.
# TODO: Just move all of Gosu's source files back into a flat hierarchy again.
# The nested directory structure has really not been worth it.
SOURCE_FILES.each do |file|
  shim_name = file.gsub('/', '-').sub(/\.mm$/, '.cpp')
  File.open(shim_name, "w") do |shim|
    shim.puts "#include \"../../src/#{file}\""
  end
end

# This is necessary to build on stock Ruby on OS X 10.7.
CONFIG['CXXFLAGS'] ||= $CXXFLAGS

if RUBY_VERSION >= '1.9.3' and RUBY_VERSION < '2.2.0' then
  # In some versions of Ruby and mkmf, the $CXXFLAGS variable is badly broken.
  # We can modify CONFIG instead, and our changes will end up in the Makefile.
  # See http://bugs.ruby-lang.org/issues/8315
  # The lower bound was reduced to 1.9.3 here: https://github.com/gosu/gosu/issues/321
  CONFIG['CXXFLAGS'] = "#$CFLAGS #$CXXFLAGS"
end

create_makefile 'gosu'
