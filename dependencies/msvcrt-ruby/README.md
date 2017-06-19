# Ruby Header/Library Files for Microsoft Visual C++

## Background

Gosu (the C++ library) and its Ruby extension are compiled using Microsoft
Visual C++. This requires header files as well as a library in the `.lib`
format.

Binaries from https://ruby-lang.org/ (up to Ruby 1.9) used to be compiled using
MSVC, and included libraries in the `.lib` format that Gosu could be linked
against.
However, all current [Ruby Installer](http://rubyinstaller.org/) builds are
compiled using MinGW, and only ship with libraries in the `.a` format, e.g.
`libmsvcrt-ruby200.dll.a`.

Luckily, the Ruby Installer packages include a DLL that we can use to generate
a `.lib` file like so:

https://adrianhenke.wordpress.com/2008/12/05/create-lib-file-from-dll/

## Step by Step

So for a new version x.y of Ruby, these are the steps to add support in Gosu:

  * Download and extract a Ruby Installer package.
  * Run the steps outlined in the first article above on the DLL file in `bin`.
  * Copy the resulting `.lib` file into *this* folder, and the contents of the
    `include` directory into `include/ruby-x.y`.
  * Create a new build configuration in `RubyGosu.vcproj` called "Release
    (Ruby 2.x)".
    Adjust the include paths and linker input file to match the new `.lib` file
    and `include` folder.
    Also adjust the output path to be `..\lib\x.y`.

## Git all the files!

To make it easier to compile Gosu on a fresh system, both header and `.lib`
files have been organised in this folder.

The copyright for all these files remains with the Ruby team and Ruby
Installer contributors.
