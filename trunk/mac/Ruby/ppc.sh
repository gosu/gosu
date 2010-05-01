#/bin/bash

mkdir -p mac/Ruby/ppc
cd mac/Ruby/ppc
export LDFLAGS="-arch ppc"
export CFLAGS="-O3 -g -isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch ppc -mmacosx-version-min=10.4 -I/Developer/SDKs/MacOSX10.4u.sdk/usr/lib/gcc/powerpc-apple-darwin10/4.0.1/include"
../source/configure --enable-shared --build=powerpc-apple-darwin8.0
ruby -e 'ls = File.readlines("Makefile"); ls.each { |l| l.gsub!(/ARCH_FLAG =/, "ARCH_FLAG = -arch ppc") }; File.open("Makefile", "w") { |f| ls.each { |l| f.puts l } }'
make
install_name_tool -id @executable_path/../Frameworks/libruby.1.9.1.dylib libruby.1.9.1.dylib
