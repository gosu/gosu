Pod::Spec.new do |s|
  s.name         = "Gosu"
  s.version      = "0.10.5"
  s.summary      = "2D game development library."
  s.homepage     = "http://libgosu.org/"
  s.documentation_url = 'http://libgosu.org/cpp/'
  
  s.license      = { :type => 'MIT', :file => 'COPYING' }
  s.author       = { "Julian Raschke" => "julian@raschke.de" }

  s.source       = { :git => "https://github.com/jlnr/gosu.git", :tag => "v0.10.5" }
  
  s.compiler_flags = '-DGOSU_DEPRECATED= -Wno-conversion'
  
  s.subspec 'Gosu' do |ss|
    ss.frameworks = 'OpenGL', 'OpenAL', 'IOKit', 'Carbon', 'Cocoa', 'AudioToolbox', 'ApplicationServices', 'ForceFeedback', 'AudioUnit', 'CoreAudio'
    ss.libraries = 'iconv'
    ss.compiler_flags = '-I/usr/local/include -I/usr/local/include/SDL2'
    ss.xcconfig = { 'OTHER_LDFLAGS' => '/usr/local/lib/libSDL2.a' }
    
    ss.public_header_files = 'Gosu/*.hpp'
    ss.source_files = ['Gosu/*.hpp', 'src/**/*.{h,hpp}'] +
      %w(src/Audio/Audio.mm
         src/Bitmap/Bitmap.cpp
         src/Bitmap/BitmapIO.cpp
         src/DirectoriesApple.mm
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
         src/TimingApple.cpp
         src/Utility.cpp
         src/UtilityApple.mm
         src/Window.cpp
         
         src/stb_vorbis.c)
    
    # This path needs to be preserved because it is included by Audio.mm (yuck)
    ss.preserve_paths = 'src/Audio/Audio.cpp'
    
    ss.requires_arc = false
    
    ss.platform = :osx, '10.7'
  end
  
  # TODO - use multi-platform support for this
  # http://docs.cocoapods.org/specification.html#ios
  s.subspec 'MobileGosu' do |ss|
    ss.frameworks = 'CoreGraphics', 'OpenGLES', 'OpenAL', 'AudioToolbox', 'AVFoundation', 'QuartzCore'
    ss.libraries = 'iconv'
    
    ss.public_header_files = 'Gosu/*.hpp'
    ss.source_files = ['Gosu/*.hpp', 'src/**/*.{h,hpp}'] +
      %w(src/Audio/Audio.mm
         src/Bitmap/Bitmap.cpp
         src/Bitmap/BitmapIO.cpp
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
         src/WindowUIKit.mm
         
         src/stb_vorbis.c)
    
    # This path needs to be preserved because it is included by Audio.mm (yuck)
    ss.preserve_paths = 'src/Audio/Audio.cpp'
    
    ss.requires_arc = false

    ss.platform = :ios, '5.1.1'
  end
  
  s.subspec 'MobileGosuMain' do |ss|
    ss.dependency 'Gosu/MobileGosu' 
    
    ss.source_files =
      %w(src/UIKit/GosuAppDelegate.mm)
    
    ss.requires_arc = false
    
    ss.platform = :ios, '5.1.1'
  end
end
