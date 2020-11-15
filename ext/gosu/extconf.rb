#!/usr/bin/env ruby

if RUBY_PLATFORM =~ /mswin$|mingw32|mingw64|win32\-|\-win32/
  platform = (RUBY_PLATFORM =~ /^x64-/ ? 'x64-mingw32' : 'i386-mingw32')
  
  puts "This gem is not meant to be installed on Windows. Instead, please use:"
  puts "gem install gosu --platform=#{platform}"
  exit 1
end

puts 'The Gosu gem requires some libraries to be installed system-wide.'
puts 'See the following site for a list:'
if `uname`.chomp == 'Darwin'
  puts 'https://github.com/gosu/gosu/wiki/Getting-Started-on-OS-X'
else
  puts 'https://github.com/gosu/gosu/wiki/Getting-Started-on-Linux'
end

require 'mkmf'
require 'fileutils'

# Silence internal deprecation warnings in Gosu.
$CFLAGS << " -DGOSU_DEPRECATED="

$CXXFLAGS ||= ""
$CXXFLAGS << " -std=gnu++11"

# Make Gosu's own header files and all of its dependencies available to C++ source files.
$INCFLAGS << " -I../../include -I../../dependencies/stb -I../../dependencies/utf8proc -I../../dependencies/SDL_sound"

if `uname`.chomp == 'Darwin'
  # Disable assertions in C code.
  $CFLAGS   << " -DNDEBUG"
  # Compile all C++ files as Objective C++ on macOS since mkmf does not support .mm files.
  $CXXFLAGS << " -x objective-c++ -fobjc-arc -DNDEBUG"
  # Prevents issues with Apple's pre-installed Ruby 2.3 on macOS 10.13.
  # https://github.com/gosu/gosu/issues/424
  $CXXFLAGS << " -Wno-reserved-user-defined-literal"

  # Explicitly specify libc++ as the standard library.
  # rvm will sometimes try to override this:
  # https://github.com/shawn42/gamebox/issues/96
  $CXXFLAGS << " -stdlib=libc++"
  
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

# Gem::Version#initialize is apparently broken in some versions of Ruby, so use a local helper.
def ruby_newer_than?(want_version)
  have_parts = RUBY_VERSION.split('.').map { |part| part.to_i }
  want_parts = want_version.split('.').map { |part| part.to_i }
  have_parts << 0 while have_parts.size < want_parts.size
  want_parts << 0 while want_parts.size < have_parts.size
  (have_parts <=> want_parts) == 1
end

# In some versions of Ruby/mkmf, the $CXXFLAGS variable does not work.
# We can modify CONFIG instead, and our changes will end up in the Makefile.
# See: http://bugs.ruby-lang.org/issues/8315
# The lower bound was reduced to 1.9.3 here: https://github.com/gosu/gosu/issues/321
if ruby_newer_than?("1.9.2") and not ruby_newer_than?("2.2.0")
  CONFIG['CXXFLAGS'] = "#$CFLAGS #$CXXFLAGS"
end

create_makefile "gosu", "../../src"
