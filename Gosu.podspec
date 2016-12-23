Pod::Spec.new do |s|
  s.name         = "Gosu"
  s.version      = "0.10.8"
  s.summary      = "2D game development library."
  s.homepage     = "https://www.libgosu.org/"
  s.documentation_url = 'https://www.libgosu.org/cpp/'
  
  s.license      = { :type => 'MIT', :file => 'COPYING' }
  s.author       = { "Julian Raschke" => "julian@raschke.de" }

  s.source       = { :git => "https://github.com/gosu/gosu.git", :tag => "v0.10.8" }

  s.subspec 'Gosu' do |ss|
    ss.osx.deployment_target = '10.7'
    ss.ios.deployment_target = '5.1.1'
    ss.requires_arc          = true
  
    # Ignore Gosu using deprecated Gosu APIs internally.
    # Compile all .cpp files as Objective-C++ so we can use ObjC frameworks.
    ss.compiler_flags = '-DGOSU_DEPRECATED= -x objective-c++'
  
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
    ss.source_files = ['Gosu/*.hpp', 'src/*.{h,hpp,c,cpp}']
   
    # This path needs to be preserved because it is included by Audio.mm (yuck)
    ss.preserve_paths = 'src/Audio.cpp'
  end
  
  s.subspec 'GosuAppDelegateMain' do |ss|
    ss.dependency 'Gosu/Gosu'
    ss.source_files = 'src/GosuAppDelegate.mm'
    ss.platform = :ios, '5.1.1'
  end

  s.default_subspec = 'Gosu'
end
