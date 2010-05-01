#/bin/bash

mkdir -p x86_64
cd x86_64
export LDFLAGS="-arch x86_64"
export CFLAGS="-O3 -g -isysroot /Developer/SDKs/MacOSX10.6.sdk -arch x86_64 -mmacosx-version-min=10.6"
../ruby-1.9.2-preview1/configure --enable-shared
ruby -e 'ls = File.readlines("Makefile"); ls.each { |l| l.gsub!(/ARCH_FLAG =/, "ARCH_FLAG = -arch x86_64") }; File.open("Makefile", "w") { |f| ls.each { |l| f.puts l } }'
make

