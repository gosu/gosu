#/bin/bash

mkdir -p mac/Ruby/i386
cd mac/Ruby/i386
export LDFLAGS="-arch i386"
export CFLAGS="-O3 -g -isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch i386 -mmacosx-version-min=10.4 -I/Developer/SDKs/MacOSX10.4u.sdk/usr/lib/gcc/i686-apple-darwin10/4.0.1/include"
../source/configure --enable-shared --build=i386-apple-darwin8.0
ruby -e 'ls = File.readlines("Makefile"); ls.each { |l| l.gsub!(/ARCH_FLAG =/, "ARCH_FLAG = -arch i386") }; File.open("Makefile", "w") { |f| ls.each { |l| f.puts l } }'
make
install_name_tool -id @executable_path/../Frameworks/libruby.1.9.1.dylib libruby.1.9.1.dylib
