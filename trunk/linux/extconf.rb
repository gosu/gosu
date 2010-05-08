#!/usr/bin/env ruby

if defined? RUBY_PLATFORM and (RUBY_PLATFORM['-win32'] or RUBY_PLATFORM['win32-'] or RUBY_PLATFORM['-mingw32']) then
  puts 'This gem is not meant to be installed on Windows. If you see this message, gem made a mistake.'
  puts 'On Windows, please use:'
  puts 'gem install gosu --platform=mswin32'
  exit 1
end

puts 'The Gosu gem requires some libraries to be installed system-wide.'
puts 'See the following site for a list:'
puts 'http://code.google.com/p/gosu/wiki/GettingStartedOnLinux'
puts

SOURCE_FILES =
    %w(Math.cpp Utility.cpp IO.cpp FileUnix.cpp InputX.cpp TextInputX.cpp TimingUnix.cpp WindowX.cpp
	     Graphics/Bitmap.cpp Graphics/BitmapUtils.cpp Graphics/Color.cpp
	     Graphics/TexChunk.cpp Graphics/Graphics.cpp Graphics/Image.cpp
	     Graphics/RotFlip.cpp Graphics/BlockAllocator.cpp
	     Graphics/Texture.cpp Graphics/LargeImageData.cpp
	     Graphics/BitmapPNG.cpp Graphics/Font.cpp Graphics/BitmapBMP.cpp
	     Graphics/TextPangoFT.cpp Graphics/Text.cpp
	     Graphics/BitmapColorKey.cpp DirectoriesUnix.cpp
	     Audio/AudioSDL.cpp RubyGosu_wrap.cxx)

require 'mkmf'

# Copy all relevant C++ files into the current directory # FIXME & LULZ
SOURCE_FILES.each { |file| `cp ../GosuImpl/#{file} #{File.basename(file)}` }

sdl_config = with_config("sdl-config", "sdl-config")
pango_config = "pkg-config pangoft2" # FIXME should probably use with_config

$INCFLAGS << " -I../ -I../GosuImpl `#{sdl_config} --cflags` `#{pango_config} --cflags`"
$LDFLAGS << " `#{pango_config} --libs`"
have_header('SDL_mixer.h') if have_library('SDL_mixer','Mix_OpenAudio')

create_makefile("gosu")

