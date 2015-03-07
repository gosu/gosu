# TODO:
# - Gosu Touch subspec for iOS
# - Make the part of Gosu Touch that wraps main() yet another subspec, so users
#   can keep their main() function and still use GosuView

Pod::Spec.new do |s|
  s.name         = "Gosu"
  s.version      = "0.0.1"
  s.summary      = "2D game development library."
  s.homepage     = "http://libgosu.org/"
  s.documentation_url = 'http://libgosu.org/cpp/'
  
  s.license      = { :type => 'MIT', :file => 'COPYING' }
  s.author       = { "Julian Raschke" => "julian@raschke.de" }

  s.source       = { :git => "https://github.com/jlnr/gosu.git" }
  
  s.compiler_flags = '-DGOSU_DEPRECATED= -Wno-conversion'
  
  s.subspec 'libogg' do |ss|
    ss.header_dir = 'ogg'
    
    ss.compiler_flags = '-Wno-comment -Wno-unused-variable -Wno-shift-op-parentheses -Wno-shorten-64-to-32'
    
    ss.public_header_files = 'dependencies/libogg/include/ogg'
    ss.source_files = 'dependencies/libogg/include/ogg', 'dependencies/libogg/src'
  end
  
  s.subspec 'libvorbis' do |ss|
    ss.dependency 'Gosu/libogg'
    
    ss.compiler_flags = '-Wno-comment -Wno-unused-variable -Wno-shift-op-parentheses -Wno-shorten-64-to-32'
    
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
    
    ss.frameworks = 'OpenGL', 'OpenAL', 'IOKit', 'Carbon', 'Cocoa', 'AudioToolbox', 'ApplicationServices', 'ForceFeedback', 'AudioUnit', 'CoreAudio'
    ss.libraries  = 'iconv'
    ss.compiler_flags = '-I/usr/local/include'
    ss.xcconfig = { 'OTHER_LDFLAGS' => '/usr/local/lib/libSDL2.a' }
    
    ss.public_header_files = 'Gosu/*.hpp'
    ss.source_files = ['Gosu/*.hpp', 'src/**/*.hpp'] +
      %w(Audio/AudioOpenAL.mm
         Bitmap/Bitmap.cpp
         Bitmap/BitmapApple.mm
         Bitmap/BitmapColorKey.cpp
         Bitmap/BitmapUtils.cpp
         DirectoriesMac.mm
         DirectoriesUnix.cpp
         FileUnix.cpp
         Graphics/BlockAllocator.cpp
         Graphics/Color.cpp
         Graphics/Graphics.cpp
         Graphics/Image.cpp
         Graphics/LargeImageData.cpp
         Graphics/Resolution.cpp
         Graphics/ResolutionApple.mm
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
         Text/TextMac.cpp
         Text/TextApple.mm
         TimingApple.cpp
         Utility.cpp
         UtilityApple.mm
         Window.cpp).map { |basename| "src/#{basename}" }
    # TODO add sockets too
    
    # This path needs to be preserved because it is included by AudioOpenAL.mm (yuck)
    ss.preserve_paths = 'src/Audio/AudioOpenAL.cpp'
    
    ss.requires_arc = false
    
    ss.platform = :osx, '10.7'
  end
  
  # TODO - use multi-platform support for this
  # TODO - This is completely outdated anyway
  # http://docs.cocoapods.org/specification.html#ios
  s.subspec 'GosuTouch' do |ss|
    ss.dependency 'Gosu/libvorbis' 

    ss.frameworks = 'CoreGraphics', 'OpenGLES', 'OpenAL', 'AudioToolbox', 'AVFoundation', 'QuartzCore'
    ss.library   = 'iconv'
    
    ss.public_header_files = 'Gosu/*.hpp'
    ss.source_files = ['Gosu/*.hpp', 'src/**/*.hpp'] +
      %w(Audio/AudioOpenAL.mm
         Bitmap/Bitmap.cpp
         Bitmap/BitmapApple.mm
         Bitmap/BitmapBMP.cpp
         Bitmap/BitmapColorKey.cpp
         Bitmap/BitmapUtils.cpp
         DirectoriesTouch.mm
         FileUnix.cpp
         GosuView.mm
         Graphics/BlockAllocator.cpp
         Graphics/Color.cpp
         Graphics/Graphics.cpp
         Graphics/Image.cpp
         Graphics/LargeImageData.cpp
         Graphics/TexChunk.cpp
         Graphics/Texture.cpp
         Graphics/Transform.cpp
         Input/AccelerometerReader.mm
         Input/InputTouch.mm
         Inspection.cpp
         IO.cpp
         Math.cpp
         Text/Font.cpp
         Text/Text.cpp
         Text/TextApple.mm
         TimingApple.cpp
         Utility.cpp
         UtilityApple.mm
         WindowTouch.mm).map { |basename| "src/#{basename}" }
    # TODO add sockets too
    ss.preserve_paths = 'src/Audio/AudioOpenAL.cpp'
    
    ss.requires_arc = false

    ss.platform = :ios, '4.3'
  end
end
