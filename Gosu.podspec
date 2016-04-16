Pod::Spec.new do |s|
  s.name         = "Gosu"
  s.version      = "0.10.6"
  s.summary      = "2D game development library."
  s.homepage     = "http://libgosu.org/"
  s.documentation_url = 'http://libgosu.org/cpp/'
  
  s.license      = { :type => 'MIT', :file => 'COPYING' }
  s.author       = { "Julian Raschke" => "julian@raschke.de" }

  s.source       = { :git => "https://github.com/jlnr/gosu.git", :tag => "v0.10.6" }

  s.osx.deployment_target = '10.7'
  s.ios.deployment_target = '5.1.1'
  s.requires_arc          = true
  
  # Ignore Gosu using deprecated Gosu APIs internally.
  s.compiler_flags = '-DGOSU_DEPRECATED= -Wno-conversion'
  
  s.libraries      = 'iconv'
  s.frameworks     = 'AudioToolbox', 'OpenAL'
  # Include all frameworks necessary for SDL 2, because we link to it statically.
  s.osx.frameworks = 'ApplicationServices', 'AudioUnit', 'Carbon', 'Cocoa', 'CoreAudio', 'ForceFeedback', 'IOKit', 'OpenGL'
  # Frameworks used directly by Gosu for iOS.
  s.ios.frameworks = 'AVFoundation', 'CoreGraphics', 'OpenGLES', 'QuartzCore'
  
  # Statically link SDL 2, so that our binary is self-contained.
  s.osx.compiler_flags = '-I/usr/local/include -I/usr/local/include/SDL2'
  s.osx.xcconfig       = { 'OTHER_LDFLAGS' => '/usr/local/lib/libSDL2.a' }
  
  s.public_header_files = 'Gosu/*.hpp'
  s.source_files = ['Gosu/*.hpp', 'src/**/*.{h,hpp}'] +
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

  s.osx.source_files =
    %w(src/Graphics/Resolution.cpp
       src/Graphics/ResolutionApple.mm
       src/Input/Input.cpp
       src/Input/TextInput.cpp
       src/Window.cpp)
   
  s.ios.source_files =
    %w(src/Input/InputUIKit.mm
       src/Inspection.cpp
       src/UIKit/GosuGLView.mm
       src/UIKit/GosuViewController.mm
       src/WindowUIKit.mm)
   
  # This path needs to be preserved because it is included by Audio.mm (yuck)
  s.preserve_paths = 'src/Audio/Audio.cpp'
  
  s.subspec 'GosuAppDelegateMain' do |ss|
    ss.source_files = 'src/UIKit/GosuAppDelegate.mm'
    ss.platform = :ios, '5.1.1'
  end
end
