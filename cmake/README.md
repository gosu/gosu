# CMake build files

These files can be used to build Gosu on Linux and OS X. The typical out-of-source build dance is:

    mkdir build
    cd build
    cmake ..
    make
    [sudo] make install

Note that if you are using OS X, it is usually easier to [use CocoaPods to add the Gosu source files to your Xcode project](https://github.com/gosu/gosu/wiki/Getting-Started-on-OS-X).

## Windows support

Even though some CMake files have checks for Windows, they have not been tested on Windows in a while and may need some work. Contributions are welcome!

In any case, the CMake files cannot replace the Visual C++ project files as long as [RubyGosu.vcproj does all the heavy lifting when compiling the Ruby/Gosu gem](https://github.com/gosu/gosu/blob/master/windows/RubyGosu.vcxproj).
