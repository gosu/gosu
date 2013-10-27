#!/usr/bin/env ruby

WINDOWS_HINTS = %w(-win32 win32- mswin mingw32)

if defined? RUBY_PLATFORM and WINDOWS_HINTS.any? { |hint| RUBY_PLATFORM.include? hint } then
  puts 'This gem is not meant to be installed on Windows. Instead, please use:'
  puts 'gem install gosu --platform=i386-mingw32'
  exit 1
end

puts 'The Gosu gem requires some libraries to be installed system-wide.'
puts 'See the following site for a list:'
puts 'https://github.com/jlnr/gosu/wiki/Getting-Started-on-Linux'
puts

BASE_FILES = %w(
  DirectoriesUnix.cpp
  FileUnix.cpp
  Graphics/Bitmap.cpp
  Graphics/BitmapColorKey.cpp
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
  UtilityApple.mm
  WindowMac.mm
)

LINUX_FILES = %w(
  Audio/AudioOpenAL.cpp
  Graphics/BitmapFreeImage.cpp
  Graphics/TextUnix.cpp
  InputX.cpp
  TextInputX.cpp
  TimingUnix.cpp
  WindowX.cpp
)

OGG_VORBIS_FILES = Dir['../dependencies/libogg/src/*.c'] +
                   Dir['../dependencies/libvorbis/lib/vorbisfile.c'] +
                   %w(analysis bitrate block codebook envelope floor0 floor1
                      info lookup lpc lsp mapping0 mdct psy registry res0
                      sharedbook smallft synthesis window).map do |basename|
                     "../dependencies/libvorbis/lib/#{basename}.c"
                   end

require 'mkmf'
require 'fileutils'

# Silence internal deprecation warnings in Gosu
$CFLAGS << " -DGOSU_DEPRECATED="

$INCFLAGS << " -I../ -I../GosuImpl"

if `uname`.chomp == 'Darwin' then
  SOURCE_FILES = BASE_FILES + MAC_FILES + OGG_VORBIS_FILES
  
  # Apple curiously distributes libpng only inside X11
  $INCFLAGS  << " -I/usr/X11/include"
  # Use included libogg, libvorbis to make Gosu easier to install on OS X
  $INCFLAGS  << " -I../dependencies/libogg/include"
  $INCFLAGS  << " -I../dependencies/libvorbis/include"
  $INCFLAGS  << " -I../dependencies/libvorbis/lib"
  # To make everything work with the Objective C runtime
  $CFLAGS    << " -x objective-c -DNDEBUG"
  # Compile all C++ files as Objective C++ on OS X since mkmf does not support .mm
  # files.
  # Also undefine two debug flags that cause exceptions to randomly crash
  # otherwise; see:
  # https://trac.macports.org/ticket/27237#comment:21
  # http://newartisans.com/2009/10/a-c-gotcha-on-snow-leopard/#comment-893
  CONFIG['CXXFLAGS'] = "#{CONFIG['CXXFLAGS']} -x objective-c++ -U_GLIBCXX_DEBUG -U_GLIBCXX_DEBUG_PEDANTIC"
  if `uname -r`.to_i >= 13 then
    # Use C++11 on Mavericks and above
    # TODO: This can probably be enabled starting from 10.6?
    CONFIG['CXXFLAGS'] << " -std=gnu++11"
  end
  $LDFLAGS   << " -L/usr/X11/lib -liconv"
  %w(AudioToolbox IOKit OpenAL OpenGL AppKit ApplicationServices Foundation Carbon).each do |f|
    $LDFLAGS << " -framework #{f}"
  end
else
  SOURCE_FILES = BASE_FILES + LINUX_FILES

  pkg_config 'sdl'
  pkg_config 'pangoft2'
  pkg_config 'x11'
  pkg_config 'xinerama'
  
  pkg_config 'gl'
  pkg_config 'vorbisfile'
  pkg_config 'openal'
  pkg_config 'sndfile'
  
  have_header 'SDL_ttf.h'   if have_library('SDL_ttf', 'TTF_RenderUTF8_Blended')
  have_header 'FreeImage.h' if have_library('freeimage', 'FreeImage_ConvertFromRawBits')
  have_header 'AL/al.h'     if have_library('openal')
end

# Symlink our pretty gosu.so into ../lib
# TODO gosu.rb should just look in the right place.
unless File.exist? "../lib/gosu.#{RbConfig::CONFIG['DLEXT']}"
  FileUtils.ln_s "../linux/gosu.#{RbConfig::CONFIG['DLEXT']}",
                 "../lib/gosu.#{RbConfig::CONFIG['DLEXT']}"
end

# And now it gets ridiculous (or I am overcomplicating things...):
# mkmf will compile all .c/.cpp files in this directory, but if they are nested
# inside folders, it will not find the resulting .o files during linking.
# So we create a shim .c/.cpp file for each file that we want to compile, ensuring
# that all .o files are built into the current directory, without any nesting.
# TODO - would be nicer if the Rakefile would just create these shim files and
# ship them along with the gem
SOURCE_FILES.each do |file|
  shim_name = File.basename(file).sub(/\.mm$/, '.cpp')
  File.open(shim_name, "w") do |shim|
    shim.puts "#include \"../GosuImpl/#{file}\""
  end
end

if RUBY_VERSION >= '2.0.0' then
  # See http://bugs.ruby-lang.org/issues/8315
  CONFIG['CXXFLAGS'] = "#$CFLAGS #{CONFIG['CXXFLAGS']}"
end

create_makefile 'gosu'
