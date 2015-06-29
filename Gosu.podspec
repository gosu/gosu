# TODO:
# - Make the part of GosuTouch that wraps main() yet another subspec, so users
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
    ss.libraries = 'iconv'
    ss.compiler_flags = '-I/usr/local/include'
    ss.xcconfig = { 'OTHER_LDFLAGS' => '/usr/local/lib/libSDL2.a' }
    
    ss.public_header_files = 'Gosu/*.hpp'
    ss.source_files = ['Gosu/*.hpp', 'src/**/*.{h,hpp}'] +
      %w(src/Audio/AudioOpenAL.mm
         src/Bitmap/Bitmap.cpp
         src/Bitmap/BitmapApple.mm
         src/Bitmap/BitmapColorKey.cpp
         src/Bitmap/BitmapUtils.cpp
         src/DirectoriesApple.mm
         src/DirectoriesUnix.cpp
         src/FileUnix.cpp
         src/Graphics/BlockAllocator.cpp
         src/Graphics/Color.cpp
         src/Graphics/Graphics.cpp
         src/Graphics/Image.cpp
         src/Graphics/LargeImageData.cpp
         src/Graphics/Macro.cpp
         src/Graphics/Resolution.cpp
         src/Graphics/ResolutionApple.mm
         src/Graphics/TexChunk.cpp
         src/Graphics/Texture.cpp
         src/Graphics/Transform.cpp
         src/Input/Input.cpp
         src/Input/TextInput.cpp
         src/Inspection.cpp
         src/IO.cpp
         src/Math.cpp
         src/Text/Font.cpp
         src/Text/Text.cpp
         src/Text/TextApple.mm
         src/Text/TextMac.cpp
         src/TimingApple.cpp
         src/Utility.cpp
         src/UtilityApple.mm
         src/Window.cpp)
         # TODO add sockets
    
    # This path needs to be preserved because it is included by AudioOpenAL.mm (yuck)
    ss.preserve_paths = 'src/Audio/AudioOpenAL.cpp'
    
    ss.requires_arc = false
    
    ss.platform = :osx, '10.7'
  end
  
  # TODO - use multi-platform support for this
  # http://docs.cocoapods.org/specification.html#ios
  s.subspec 'GosuTouch' do |ss|
    ss.dependency 'Gosu/libvorbis' 

    ss.frameworks = 'CoreGraphics', 'OpenGLES', 'OpenAL', 'AudioToolbox', 'AVFoundation', 'QuartzCore'
    ss.libraries = 'iconv'
    
    ss.public_header_files = 'Gosu/*.hpp'
    ss.source_files = ['Gosu/*.hpp', 'src/**/*.{h,hpp}'] +
      %w(src/Audio/AudioOpenAL.mm
         src/Bitmap/Bitmap.cpp
         src/Bitmap/BitmapApple.mm
         src/Bitmap/BitmapBMP.cpp
         src/Bitmap/BitmapColorKey.cpp
         src/Bitmap/BitmapUtils.cpp
         src/DirectoriesApple.mm
         src/FileUnix.cpp
         src/Graphics/BlockAllocator.cpp
         src/Graphics/Color.cpp
         src/Graphics/Graphics.cpp
         src/Graphics/Image.cpp
         src/Graphics/LargeImageData.cpp
         src/Graphics/Macro.cpp
         src/Graphics/TexChunk.cpp
         src/Graphics/Texture.cpp
         src/Graphics/Transform.cpp
         src/Input/InputUIKit.mm
         src/Inspection.cpp
         src/IO.cpp
         src/Math.cpp
         src/Text/Font.cpp
         src/Text/Text.cpp
         src/Text/TextApple.mm
         src/TimingApple.cpp
         src/UIKit/GosuAppDelegate.mm
         src/UIKit/GosuGLView.mm
         src/UIKit/GosuViewController.mm
         src/Utility.cpp
         src/UtilityApple.mm
         src/WindowUIKit.mm)
         # TODO add sockets
    
    ss.preserve_paths = 'src/Audio/AudioOpenAL.cpp'
    
    ss.requires_arc = false

    ss.platform = :ios, '5.1.1'
  end
end
