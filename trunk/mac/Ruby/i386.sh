#/bin/bash

mkdir -p i386
cd i386
export LDFLAGS="-arch i386"
export CFLAGS="-O3 -g -isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch i386 -mmacosx-version-min=10.4 -I/Developer/SDKs/MacOSX10.4u.sdk/usr/lib/gcc/i686-apple-darwin10/4.0.1/include"
../ruby-1.9.2-preview1/configure --enable-shared
ruby -e 'ls = File.readlines("Makefile"); ls.each { |l| l.gsub!(/ARCH_FLAG =/, "ARCH_FLAG = -arch i386") }; File.open("Makefile", "w") { |f| ls.each { |l| f.puts l } }'
make
