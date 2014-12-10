# Ruby Header/Library Files for Microsoft Visual C++

## Background

Gosu (the C++ library) and its Ruby extension are compiled using Microsoft
Visual C++. This requires header files as well as a library in the `.lib`
format.

Binaries from https://ruby-lang.org/ (up to Ruby 1.9) were compiled using MSVC,
and included libraries in the `.lib` format that Gosu could be linked against.
However, all current [Ruby Installer](http://rubyinstaller.org/) builds are
compiled using MinGW, and only ship with libraries in the `.a` format, e.g.
`libmsvcrt-ruby200.dll.a`.

Thankfully, the Ruby Installer packages include a DLL that we can use to
generate a `.lib` file like so:

https://adrianhenke.wordpress.com/2008/12/05/create-lib-file-from-dll/

(Note: For 64-bit versions, it seems to work if you follow these steps from a
Windows SDK 7.1 command prompt, but not from the 32-bit Visual C++ Express
command prompt, even when using `/MACHINE:x64`.

We can also use most of the headers included in the Ruby Installer packages.
However we need a platform- and compiler-specific `ruby/config.h` for our
version of Visual C++.
For this, it is enough to start compilation of Ruby on Windows from a Visual C++
command prompt, like this:

http://blog.cyplo.net/2011/01/01/compiling-ruby-1-9-2-windows/

## Step by Step

So for a new version x.y.z of Ruby, these are the steps to add support in Gosu:

  * Download and extract a Ruby Installer package.
  * Run the steps outlined in the first article above on the DLL file in `bin`.
  * Copy the resulting `.lib` file and the `include` directory into *this*
    directory, where the `include` folder should be renamed `ruby-x.y.z`.
  * Download and extract the matching source tarball from
    https://www.ruby-lang.org/en/downloads/.
  * In the `win32` folder, run `configure.bat && nmake`.
    Even if compilation fails, there should be a `.ext` folder in `win32` that
    includes the compiler-specific `ruby/config.h` header.
    Copy its parent directory into `ruby-x.y.z`.
  * Create a new build configuration in `RubyGosu.vcproj` called "Release
    (Ruby 2.x)".
    Adjust the include paths and linker input file to match the new `.lib` file
    and `include` folder.
    Also adjust the output path to be `..\lib\x.y`.

## Git all the files!

To make it easier to compile Gosu on a fresh system, both header and `.lib`
files (either generated or downloaded from ruby-lang.org) have been organised in
this folder.

The copyright for all these files remains with the Ruby team and Ruby
Installer contributors.
