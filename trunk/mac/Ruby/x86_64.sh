#/bin/bash

mkdir -p mac/Ruby/x86_64
cd mac/Ruby/x86_64
export LDFLAGS="-arch x86_64"
export CFLAGS="-O3 -g -isysroot /Developer/SDKs/MacOSX10.6.sdk -arch x86_64 -mmacosx-version-min=10.6"
../source/configure --enable-shared --build=x86_64-darwin10.0
ruby -e 'ls = File.readlines("Makefile"); ls.each { |l| l.gsub!(/ARCH_FLAG =/, "ARCH_FLAG = -arch x86_64") }; File.open("Makefile", "w") { |f| ls.each { |l| f.puts l } }'
make
install_name_tool -id @executable_path/../Frameworks/libruby.1.9.1.dylib libruby.1.9.1.dylib
