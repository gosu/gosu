require "rbconfig"

case RbConfig::CONFIG["host_os"]
when /^mingw/
  windows = true
when /^darwin/
  macos = true
end

if not windows
  puts "The Gosu gem requires some libraries to be installed system-wide."
  puts "See the following wiki page for a list:"
  if macos
    puts "https://github.com/gosu/gosu/wiki/Getting-Started-on-OS-X"
  else
    puts "https://github.com/gosu/gosu/wiki/Getting-Started-on-Linux"
  end
end

require "mkmf"
require "fileutils"

# Ensure that FFI functions are exported.
$CXXFLAGS << " -DGOSU_FFI_EXPORTS"

# Silence internal deprecation warnings in Gosu.
$CXXFLAGS << " -DGOSU_DEPRECATED="

# Disable assertions in C and C++ code.
$CFLAGS << " -DNDEBUG"
$CXXFLAGS << " -DNDEBUG"

# Enable C++20.
$CXXFLAGS << " -std=gnu++20"

# Make Gosu's own header files and all of its dependencies available to C++ source files.
%w(include dependencies/stb dependencies/utf8proc dependencies/SDL_sound dependencies/mojoAL/AL).each do |incdir|
  $INCFLAGS << " -I$(srcdir)/../../#{incdir}"
end

# We statically compile utf8proc into the Gosu binary.
# This macro is required on Windows to avoid errors about dllexport/dllimport mismatches.
# On other platforms, it might help us avoid unnecessarily exported symbols.
$CFLAGS << " -DUTF8PROC_STATIC -DAL_LIBTYPE_STATIC"
$CXXFLAGS << " -DUTF8PROC_STATIC -DAL_LIBTYPE_STATIC"

if windows
  # Use the bundled version of SDL 3.
  $INCFLAGS << " -I$(srcdir)/../../dependencies/SDL/include"
  if RbConfig::CONFIG["arch"] =~ /x64-/
    $LDFLAGS << "  -L$(srcdir)/../../dependencies/SDL/lib/x64"
  else
    $LDFLAGS << "  -L$(srcdir)/../../dependencies/SDL/lib/x86"
  end
  $LDFLAGS << " -lgdi32 -lwinmm -ldwmapi -lOpenGL32 -lSDL3"
  # Link libstdc++ statically to avoid having another DLL dependency when using Ocra.
  $LDFLAGS << " -static-libstdc++"
elsif macos
  # Compile all C++ files as Objective C++ on macOS since mkmf does not support .mm files.
  $CXXFLAGS << " -x objective-c++ -fobjc-arc"
  # Also hide all symbols by default. FFI symbols use the GOSU_FFI_API macro to override this.
  $CXXFLAGS << " -fvisibility=hidden -DGOSU_FFI_EXPORTS"

  # Explicitly specify libc++ as the standard library.
  # rvm will sometimes try to override this:
  # https://github.com/shawn42/gamebox/issues/96
  $CXXFLAGS << " -stdlib=libc++"

  # Disable an error that is enabled by default and prevents Gosu from building with universal Ruby:
  # https://trac.macports.org/ticket/58255 / https://github.com/gosu/gosu/issues/424
  $CXXFLAGS << " -Wno-reserved-user-defined-literal"

  # Dependencies. SDL3 ships no sdl3-config, so use pkg-config instead.
  $CFLAGS << " #{`pkg-config --cflags sdl3`.chomp}"
  $CXXFLAGS << " #{`pkg-config --cflags sdl3`.chomp}"
  # Prefer statically linking SDL 3.
  $LDFLAGS << " #{`pkg-config --static --libs sdl3`.chomp} -framework OpenGL -framework Metal"
else
  # Raspberry Pi support, hasn't been tested in a long time.
  if /BCM2708/ =~ `cat /proc/cpuinfo`
    $INCFLAGS << " -I/opt/vc/include/GLES"
    $INCFLAGS << " -I/opt/vc/include"
    $LDFLAGS << " -L/opt/vc/lib"
    $LDFLAGS << " -lGLESv1_CM"
    $CFLAGS << " -DGOSU_IS_OPENGLES"
    $CXXFLAGS << " -DGOSU_IS_OPENGLES"
  else
    pkg_config "gl"
  end

  pkg_config "sdl3"
  pkg_config "fontconfig"
end

# mkmf only looks for C/C++ files in $srcdir by default, but our source files are in other folders.

Dir.chdir($srcdir) do # (not needed when installing the gem, but for 'rake compile')
  $srcs = Dir["../../dependencies/**/*.c"] + Dir["../../{src,ffi}/*.cpp"]
  # We need to expand the $VPATH so that all source files can be found reliably. https://stackoverflow.com/a/35842162
  $VPATH += $srcs.map { |src| "$(srcdir)/#{File.dirname(src)}" }.uniq
end

create_makefile "gosu-ffi"
