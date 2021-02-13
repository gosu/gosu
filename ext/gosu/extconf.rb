require "rbconfig"

case RbConfig::CONFIG["host_os"]
when /^mingw/
  windows = true
when /^darwin/
  macos = true
end

if not windows
  puts 'The Gosu gem requires some libraries to be installed system-wide.'
  puts 'See the following site for a list:'
  if macos
    puts 'https://github.com/gosu/gosu/wiki/Getting-Started-on-OS-X'
  else
    puts 'https://github.com/gosu/gosu/wiki/Getting-Started-on-Linux'
  end
end

require 'mkmf'
require 'fileutils'

# Silence internal deprecation warnings in Gosu.
$CFLAGS << " -DGOSU_DEPRECATED="

# Disable assertions in C code.
$CFLAGS << " -DNDEBUG"

$CXXFLAGS ||= ""
# Enable C++17, but at the same time make the compiler more permissive to avoid this error:
# https://stackoverflow.com/a/31899029 (triggered by ruby.h, which uses register variables)
# The "register" macro can be removed when support for Ruby 2.6 and older has been dropped.
$CXXFLAGS << " -std=gnu++17 -Dregister="

# Make Gosu's own header files and all of its dependencies available to C++ source files.
$INCFLAGS << " -I../../include -I../../dependencies/stb -I../../dependencies/utf8proc -I../../dependencies/SDL_sound"

if windows
  # We statically compile utf8proc into the Gosu binary.
  # This macro is required to avoid errors about dllexport/dllimport mismatches.
  $CFLAGS << " -DUTF8PROC_STATIC"
  $CXXFLAGS << " -DUTF8PROC_STATIC"

  # Define UNICODE to use the Unicode-aware variants of all Win32 API functions.
  $CXXFLAGS << " -DUNICODE"

  # Use the bundled versions of SDL 2 and open_al.
  $INCFLAGS << " -I../../dependencies/al_soft -I../../dependencies/SDL/include"
  if RbConfig::CONFIG['arch'] =~ /x64-/
    $LDFLAGS << "  -L../../dependencies/al_soft/x64 -L../../dependencies/SDL/lib/x64"
  else
    $LDFLAGS << "  -L../../dependencies/al_soft/x86 -L../../dependencies/SDL/lib/x86"
  end
  $LDFLAGS << " -lgdi32 -lwinmm -ldwmapi -lOpenGL32 -lOpenAL32 -lSDL2"
  # Link libstdc++ statically to avoid having another DLL dependency when using Ocra.
  $LDFLAGS << " -static-libstdc++"
elsif macos
  # Compile all C++ files as Objective C++ on macOS since mkmf does not support .mm files.
  $CXXFLAGS << " -x objective-c++ -fobjc-arc -DNDEBUG"

  # Explicitly specify libc++ as the standard library.
  # rvm will sometimes try to override this:
  # https://github.com/shawn42/gamebox/issues/96
  $CXXFLAGS << " -stdlib=libc++"

  # Disable an error that is disabled by default and prevents Gosu from building with universal Ruby:
  # https://trac.macports.org/ticket/58255 / https://github.com/gosu/gosu/issues/424
  $CXXFLAGS << " -Wno-reserved-user-defined-literal"
  
  # Dependencies.
  $CFLAGS << " #{`sdl2-config --cflags`.chomp}"
  $CXXFLAGS << " #{`sdl2-config --cflags`.chomp}"
  # Prefer statically linking SDL 2.
  $LDFLAGS  << " #{`sdl2-config --static-libs`.chomp} -framework OpenGL -framework Metal -framework OpenAL"
  # And yet another hack: `sdl2-config --static-libs` uses `-lSDL2` instead of linking to the static library,
  # even if it exists. -> Manually replace it. (Ugh!)
  $LDFLAGS.sub! " -lSDL2 ", " /usr/local/lib/libSDL2.a " if File.exist? "/usr/local/lib/libSDL2.a"
  
  # Disable building of 32-bit slices in Apple's Ruby.
  # (RbConfig::CONFIG['CXXFLAGS'] on 10.11: -arch x86_64 -arch i386 -g -Os -pipe)
  $CFLAGS.gsub! "-arch i386", ""
  $CXXFLAGS.gsub! "-arch i386", ""
  $LDFLAGS.gsub! "-arch i386", ""
  $ARCH_FLAG.gsub! "-arch i386", ""
  CONFIG['LDSHARED'].gsub! "-arch i386", ""
else
  if /BCM2708/ =~ `cat /proc/cpuinfo`
    $INCFLAGS << " -I/opt/vc/include/GLES"
    $INCFLAGS << " -I/opt/vc/include"
    $LDFLAGS << " -L/opt/vc/lib"
    $LDFLAGS << " -lGLESv1_CM"
    $CFLAGS   << " -DGOSU_IS_OPENGLES"
    $CXXFLAGS << " -DGOSU_IS_OPENGLES"
  else
    pkg_config 'gl'
  end

  pkg_config 'sdl2'
  pkg_config 'vorbisfile'
  pkg_config 'openal'
  pkg_config 'sndfile'
  pkg_config 'libmpg123'
  pkg_config 'fontconfig'
  
  have_header 'AL/al.h' if have_library('openal')
end

# Because Ruby will only compile source file in the src/ folder, but our dependencies are
# stored in dependencies/, we need to create symlinks for all the .c files. This is terrible,
# but still seems better than putting everything into the same folder to begin with.
Dir.glob('../../dependencies/**/*.c').each do |dep|
  File.open("../../src/#{File.basename dep}", "w") do |shim|
    shim.puts "#include \"#{File.expand_path dep}\""
  end
end

create_makefile "gosu", "../../src"
