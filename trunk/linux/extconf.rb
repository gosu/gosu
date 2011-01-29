#!/usr/bin/env ruby

if defined? RUBY_PLATFORM and (RUBY_PLATFORM['-win32'] or RUBY_PLATFORM['win32-'] or RUBY_PLATFORM['mswin'] or RUBY_PLATFORM['mingw32']) then
  puts 'This gem is not meant to be installed on Windows. If you see this message, gem made a mistake.'
  puts 'On Windows, please use:'
  puts 'gem install gosu --platform=i386-mingw32'
  exit 1
end

puts 'The Gosu gem requires some libraries to be installed system-wide.'
puts 'See the following site for a list:'
puts 'http://code.google.com/p/gosu/wiki/GettingStartedOnLinux'
puts

# FIXME should reversely filter out files ending in Win, Mac, Touch, AL etc.
SOURCE_FILES =
    %w(Math.cpp Utility.cpp IO.cpp FileUnix.cpp InputX.cpp TextInputX.cpp TimingUnix.cpp WindowX.cpp
	     Graphics/Bitmap.cpp Graphics/BitmapUtils.cpp Graphics/Color.cpp
	     Graphics/TexChunk.cpp Graphics/Graphics.cpp Graphics/Image.cpp
	     Graphics/BlockAllocator.cpp
	     Graphics/Texture.cpp Graphics/LargeImageData.cpp
	     Graphics/BitmapPNG.cpp Graphics/Font.cpp Graphics/BitmapBMP.cpp
	     Graphics/TextUnix.cpp Graphics/Text.cpp Graphics/Transform.cpp
	     Graphics/BitmapColorKey.cpp DirectoriesUnix.cpp
	     Audio/AudioAudiere.cpp RubyGosu_wrap.cxx)

require 'mkmf'

# Copy all relevant C++ files into the current directory
# FIXME could be done by gem packager
SOURCE_FILES.each { |file| `cp ../GosuImpl/#{file} #{File.basename(file)}` }

# Symlink our pretty gosu.so into ../lib
# FIXME gosu.rb should just look in the right place
`ln -s ../linux/gosu.so ../lib/gosu.so`

sdl_config = with_config("sdl-config", "sdl-config")
audiere_config = with_config("audiere-config", "audiere-config")
pango_config = "pkg-config pangoft2" # FIXME should probably use with_config

$INCFLAGS << " -I../ -I../GosuImpl `#{sdl_config} --cflags` `#{audiere_config} --cxxflags` `#{pango_config} --cflags`"
$LDFLAGS << " `#{sdl_config} --libs` `#{audiere_config} --libs` `#{pango_config} --libs` -lX11"
have_header('SDL_ttf.h') if have_library('SDL_ttf', 'TTF_RenderUTF8_Blended')
have_header('gl.h') if have_library('GL', 'glMatrixMode')
have_header('png.h') if have_library('png', 'png_sig_cmp')

create_makefile("gosu")

