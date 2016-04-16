Pod::Spec.new do |s|
  s.name         = "Gosu"
  s.version      = "0.10.6"
  s.summary      = "2D game development library."
  s.homepage     = "http://libgosu.org/"
  s.documentation_url = 'http://libgosu.org/cpp/'
  
  s.license      = { :type => 'MIT', :file => 'COPYING' }
  s.author       = { "Julian Raschke" => "julian@raschke.de" }

  s.source       = { :git => "https://github.com/gosu/gosu.git", :tag => "v0.10.6" }

  s.subspec 'Gosu' do |ss|
    ss.osx.deployment_target = '10.7'
    ss.ios.deployment_target = '5.1.1'
    ss.requires_arc          = true
  
    # Ignore Gosu using deprecated Gosu APIs internally.
    ss.compiler_flags = '-DGOSU_DEPRECATED= -Wno-conversion'
  
    ss.libraries      = 'iconv'
    ss.frameworks     = 'AudioToolbox', 'OpenAL'
    # Include all frameworks necessary for SDL 2, because we link to it statically.
    ss.osx.frameworks = 'ApplicationServices', 'AudioUnit', 'Carbon', 'Cocoa', 'CoreAudio', 'ForceFeedback', 'IOKit', 'OpenGL'
    # Frameworks used directly by Gosu for iOS.
    ss.ios.frameworks = 'AVFoundation', 'CoreGraphics', 'OpenGLES', 'QuartzCore'
  
    # Statically link SDL 2, so that our binary is self-contained.
    ss.osx.compiler_flags = '-I/usr/local/include -I/usr/local/include/SDL2'
    ss.osx.xcconfig       = { 'OTHER_LDFLAGS' => '/usr/local/lib/libSDL2.a' }
  
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
         src/Inspection.cpp
         src/IO.cpp
         src/Math.cpp
         src/Text/Font.cpp
         src/Text/Text.cpp
         src/Text/TextApple.mm
         src/TimingApple.cpp
         src/Utility.cpp
         src/UtilityApple.mm
       
         src/stb_vorbis.c)

    ss.osx.source_files =
      %w(src/Graphics/Resolution.cpp
         src/Graphics/ResolutionApple.mm
         src/Input/Input.cpp
         src/Input/TextInput.cpp
         src/Window.cpp)
   
    ss.ios.source_files =
      %w(src/Input/InputUIKit.mm
         src/Inspection.cpp
         src/UIKit/GosuGLView.mm
         src/UIKit/GosuViewController.mm
         src/WindowUIKit.mm)
   
    # This path needs to be preserved because it is included by Audio.mm (yuck)
    ss.preserve_paths = 'src/Audio/Audio.cpp'
  end
  
  s.subspec 'GosuAppDelegateMain' do |ss|
    ss.dependency 'Gosu/Gosu'
    ss.source_files = 'src/UIKit/GosuAppDelegate.mm'
    ss.platform = :ios, '5.1.1'
  end

  s.default_subspec = 'Gosu'
end
