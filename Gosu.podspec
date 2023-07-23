SDL2_PREFIX = `sdl2-config --prefix`.chomp

Pod::Spec.new do |s|
  s.name = "Gosu"
  s.version = "1.4.6"
  s.summary = "2D game development library."
  s.homepage = "https://www.libgosu.org/"
  s.license = { type: "MIT", file: "COPYING" }
  s.author = { "Julian Raschke" => "julian@raschke.de" }
  s.documentation_url = "https://www.libgosu.org/cpp/"

  s.source = { git: "https://github.com/gosu/gosu.git", tag: "v#{s.version}" }

  # Bundle our dependencies into one subspec so that we can silence warnings for them.
  s.subspec "Dependencies" do |ss|
    ss.compiler_flags = "-Wno-everything"
    # Be careful not to include SDL_sound or mojoAL on iOS, where Gosu still uses AudioToolbox and OpenAL instead.
    ss.source_files = "dependencies/{stb,utf8proc}/**/*.{h,c}"
    ss.osx.source_files = "dependencies/{SDL_sound,mojoAL}/**/*.{h,c}"
    ss.osx.compiler_flags = "-I#{SDL2_PREFIX}/include/SDL2 -Idependencies/mojoAL/AL"
  end

  s.subspec "Gosu" do |ss|
    ss.dependency "Gosu/Dependencies"
    ss.osx.deployment_target = "10.15"
    ss.ios.deployment_target = "13.0"

    # Ignore Gosu using deprecated Gosu APIs internally.
    # Compile all source files as Objective-C++ so we can use ObjC frameworks where necessary.
    # Also silence warnings about invalid doxygen markup in SDL headers, and make deps available.
    ss.compiler_flags = "-DGOSU_DEPRECATED= -DGLES_SILENCE_DEPRECATION -Wno-documentation -x objective-c++ -Idependencies/stb -Idependencies/utf8proc"

    ss.libraries = "iconv"
    # Include all frameworks necessary for SDL 2, because we link to it statically.
    ss.osx.frameworks = "ApplicationServices", "AudioUnit", "Carbon", "Cocoa", "CoreAudio",
                        "ForceFeedback", "IOKit", "OpenGL"
    ss.osx.weak_frameworks = "CoreHaptics", "GameController", "QuartzCore", "Metal"
    # Frameworks used directly by Gosu for iOS.
    ss.ios.frameworks = "AVFoundation", "CoreGraphics", "OpenGLES", "QuartzCore", "AudioToolbox", "OpenAL"

    ss.osx.compiler_flags = "-I#{SDL2_PREFIX}/include/SDL2 -Idependencies/mojoAL"
    # Statically link SDL 2, so that compiled games will be self-contained.
    ss.osx.xcconfig = { "OTHER_LDFLAGS" => "#{SDL2_PREFIX}/lib/libSDL2.a" }

    ss.public_header_files = "include/Gosu/*.hpp"
    ss.source_files = ["include/Gosu/*.hpp", "src/*.{hpp,cpp}"]
    # Do not include FFI wrappers in the Pod project - this spec is for the C++ interface.
    ss.exclude_files = "src/Constants.cpp", "src/*Wrapper.cpp"

    # Gosu requires C++20 features and GNU extensions, but Xcode only uses gnu++14 by default.
    ss.pod_target_xcconfig = {
      "CLANG_CXX_LANGUAGE_STANDARD" => "gnu++20",
      "CLANG_CXX_LIBRARY" => "libc++",
    }
  end

  s.subspec "GosuAppDelegateMain" do |ss|
    ss.dependency "Gosu/Gosu"
    ss.source_files = "src/GosuAppDelegate.mm"
    ss.platform = :ios, "8.0"
  end

  s.default_subspec = "Gosu"
end
