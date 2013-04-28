# TODO:
# - Gosu Touch subspec for iOS
# - Make the part of Gosu Touch that wraps main() yet another subspec, so users
#   can keep their main() function and still use GosuView
# - 

Pod::Spec.new do |s|
  s.name         = "Gosu"
  s.version      = "0.0.1"
  s.summary      = "2D game development library."
  s.homepage     = "http://libgosu.org/"

  s.license      = { :type => 'MIT', :file => 'COPYING' }
  s.author       = { "Julian Raschke" => "julian@raschke.de" }

  s.source       = { :git => "https://github.com/jlnr/gosu.git", :commit => "4b52ee0f70" }

  s.subspec 'libogg' do |ss|
    ss.header_dir = 'ogg'
    ss.public_header_files = 'dependencies/libogg/include/ogg'
    ss.source_files = 'dependencies/libogg/include/ogg', 'dependencies/libogg/src'
  end
  
  s.subspec 'libvorbis' do |ss|
    ss.dependency 'Gosu/libogg'
    
    ss.header_dir = 'vorbis'
    ss.public_header_files = 'dependencies/libvorbis/include/vorbis'
    ss.source_files = %w(analysis bitrate block codebook envelope floor0 floor1
        info lookup lpc lsp mapping0 mdct psy registry res0
        sharedbook smallft synthesis vorbisfile window).map do |basename|
      "dependencies/libvorbis/lib/#{basename}.c"
    end + ['dependencies/libvorbis/include/vorbis', 'dependencies/libvorbis/lib/*.h']
  end

  s.subspec 'Gosu' do |ss|
    ss.dependency 'Gosu/libvorbis' 

    ss.frameworks = 'OpenGL', 'OpenAL', 'IOKit', 'Carbon', 'Cocoa', 'AudioToolbox', 'ApplicationServices'
    ss.library   = 'iconv'
    # To find libpng headers, TODO use compiler flags for that, does not need to leak into client project
    ss.xcconfig = { 'HEADER_SEARCH_PATHS' => '$(SDKROOT)/usr/X11/include', 'CLANG_CXX_LIBRARY' => 'libstdc++' }
    
    ss.public_header_files = 'Gosu/*.hpp'
    ss.source_files = ['Gosu/*.hpp', 'GosuImpl/**/*.hpp'] +
    # Implementation files for OS X, taken from extconf.rb
    # TODO - keep in sync with extconf & project - how?
    %w(DirectoriesUnix.cpp
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
    Utility.cpp
    Audio/AudioOpenAL.mm
    Graphics/BitmapApple.mm
    Graphics/TextMac.cpp
    Graphics/TextTouch.mm
    InputMac.mm
    TextInputMac.mm
    TimingApple.cpp
    UtilityApple.mm
    WindowMac.mm).map { |basename| "GosuImpl/#{basename}" }
    ss.preserve_files = 'GosuImpl/Audio/AudioOpenAL.cpp'
    
    ss.platform = :osx, '10.5'
  end
end
