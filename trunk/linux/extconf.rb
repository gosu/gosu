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

# Copy all relevant C++ files into the current directory (rofl)

SOURCE_FILES.each { |file| `cp ../GosuImpl/#{file} #{File.basename(file)}` }

create_makefile("gosu")

