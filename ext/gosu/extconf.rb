#!/usr/bin/env ruby

WINDOWS_HINTS = %w(-win32 win32- mswin mingw32)

if RUBY_PLATFORM =~ /mswin|mingw32|mingw64|win32\-|\-win32/ then
  puts 'This gem is not meant to be installed on Windows. Instead, please use:'
  puts 'gem install gosu --platform=i386-mingw32'
  exit 1
end

puts 'The Gosu gem requires some libraries to be installed system-wide.'
puts 'See the following site for a list:'
if `uname`.chomp == "Darwin" then
  puts 'https://github.com/jlnr/gosu/wiki/Getting-Started-on-OS-X'
else
  puts 'https://github.com/jlnr/gosu/wiki/Getting-Started-on-Linux'
end
puts

BASE_FILES = %w(
  Bitmap/Bitmap.cpp
  Bitmap/BitmapColorKey.cpp
  Bitmap/BitmapUtils.cpp
  DirectoriesUnix.cpp
  FileUnix.cpp
  Graphics/BlockAllocator.cpp
  Graphics/Color.cpp
  Graphics/Graphics.cpp
  Graphics/Image.cpp
  Graphics/LargeImageData.cpp
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
)

MAC_FILES = %w(
  Audio/AudioOpenAL.mm
  Bitmap/BitmapApple.mm
  Graphics/ResolutionApple.mm
  Text/TextApple.mm
  Text/TextMac.cpp
  TimingApple.cpp
  UtilityApple.mm
)

LINUX_FILES = %w(
  Audio/AudioOpenAL.cpp
  Bitmap/BitmapFreeImage.cpp
  Text/TextUnix.cpp
  TimingUnix.cpp
)

require 'mkmf'
require 'fileutils'

# Silence internal deprecation warnings in Gosu
$CFLAGS << " -DGOSU_DEPRECATED="

$INCFLAGS << " -I../.. -I../../src"

if `uname`.chomp == 'Darwin' then
  HOMEBREW_DEPENDENCIES = %w(SDL2 ogg vorbis vorbisfile)
  FRAMEWORKS = %w(AudioToolbox IOKit OpenAL OpenGL AppKit ApplicationServices Foundation Carbon)

  SOURCE_FILES = BASE_FILES + MAC_FILES
  
  # To make everything work with the Objective C runtime
  $CFLAGS    << " -I/usr/local/include -x objective-c -DNDEBUG"
  # Compile all C++ files as Objective C++ on OS X since mkmf does not support .mm
  # files.
  # Also undefine two debug flags that cause exceptions to randomly crash, see:
  # https://trac.macports.org/ticket/27237#comment:21
  # http://newartisans.com/2009/10/a-c-gotcha-on-snow-leopard/#comment-893
  CONFIG['CXXFLAGS'] = "#{CONFIG['CXXFLAGS']} -I/usr/local/include -x objective-c++ -U_GLIBCXX_DEBUG -U_GLIBCXX_DEBUG_PEDANTIC"
  if `uname -r`.to_i >= 13 then
    # Use C++11 on Mavericks and above
    # TODO: This can probably be enabled starting from 10.6?
    CONFIG['CXXFLAGS'] << " -std=gnu++11"
  end
  
  $LDFLAGS << " -liconv"
  
  if enable_config('static-homebrew-dependencies') then
    # TODO: For some reason this only works after deleting both SDL2 dylib files from /usr/local/lib.
    # Otherwise, the resulting gosu.bundle is still dependent on libSDL2-2.0.0.dylib, see `otool -L gosu.bundle`
    $LDFLAGS << HOMEBREW_DEPENDENCIES.map { |lib| " /usr/local/lib/lib#{lib}.a" }.join
  else
    $LDFLAGS << HOMEBREW_DEPENDENCIES.map { |lib| " -l#{lib}" }.join
  end

  $LDFLAGS << FRAMEWORKS.map { |f| " -framework #{f}" }.join
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
  have_header 'FreeImage.h' if have_library('freeimage', 'FreeImage_ConvertFromRawBits')
  have_header 'AL/al.h'     if have_library('openal')
end

# And now it gets ridiculous (or I am overcomplicating things...):
# mkmf will compile all .c/.cpp files in this directory, but if they are nested
# inside folders, it will not find the resulting .o files during linking.
# So we create a shim .c/.cpp file for each file that we want to compile, ensuring
# that all .o files are built into the current directory, without any nesting.
# TODO - would be nicer if the Rakefile would just create these shim files and
# ship them along with the gem
SOURCE_FILES.each do |file|
  shim_name = file.gsub('/', '-').sub(/\.mm$/, '.cpp')
  File.open(shim_name, "w") do |shim|
    shim.puts "#include \"../../src/#{file}\""
  end
end

if RUBY_VERSION >= '2.0.0' then
  # See http://bugs.ruby-lang.org/issues/8315
  CONFIG['CXXFLAGS'] = "#$CFLAGS #{CONFIG['CXXFLAGS']}"
end

create_makefile 'gosu'
