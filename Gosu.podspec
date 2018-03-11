Pod::Spec.new do |s|
  s.name         = "Gosu"
  s.version      = "0.13.3"
  s.summary      = "2D game development library."
  s.homepage     = "https://www.libgosu.org/"
  s.documentation_url = "https://www.libgosu.org/cpp/"
  
  s.license      = { :type => "MIT", :file => "COPYING" }
  s.author       = { "Julian Raschke" => "julian@raschke.de" }

  s.source       = { :git => "https://github.com/gosu/gosu.git", :tag => "v0.13.3" }

  s.subspec "Gosu" do |ss|
    ss.osx.deployment_target = "10.9"
    ss.ios.deployment_target = "5.1.1"
    
    # Ignore Gosu using deprecated Gosu APIs internally.
    # Compile all source files as Objective-C++ so we can use ObjC frameworks where necessary.
    # Also silence warnings about invalid doxygen markup in SDL headers.
    ss.compiler_flags = "-DGOSU_DEPRECATED= -Wno-documentation -x objective-c++"
    
    ss.libraries = "iconv"
    ss.frameworks = "AudioToolbox", "OpenAL"
    # Include all frameworks necessary for SDL 2, because we link to it statically.
    ss.osx.frameworks = "ApplicationServices", "AudioUnit", "Carbon", "Cocoa", "CoreAudio",
                        "ForceFeedback", "IOKit", "OpenGL", "Metal"
    # Frameworks used directly by Gosu for iOS.
    ss.ios.frameworks = "AVFoundation", "CoreGraphics", "OpenGLES", "QuartzCore"
    
    ss.osx.compiler_flags = "-I/usr/local/include/SDL2"
    # Statically link SDL 2, so that compiled games will be self-contained.
    ss.osx.xcconfig = { "OTHER_LDFLAGS" => "/usr/local/lib/libSDL2.a" }
    
    ss.public_header_files = "Gosu/*.hpp"
    ss.source_files = ["Gosu/*.hpp", "src/*.{h,hpp,c,cpp}"]
  end
  
  s.subspec "GosuAppDelegateMain" do |ss|
    ss.dependency "Gosu/Gosu"
    ss.source_files = "src/GosuAppDelegate.mm"
    ss.platform = :ios, "5.1.1"
  end

  s.default_subspec = "Gosu"
end
