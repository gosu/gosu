# Ruby Header/Library Files for Microsoft Visual C++

Gosu (the C++ library) and its Ruby extension are compiled using Microsoft
Visual C++, for which we need both headers and LIB files to link against.

Binaries from ruby-lang.org (up to Ruby 1.9.x) were compiled using MSVC and have
included libraries in the LIB format that Gosu could be linked against. However,
all current rubyinstaller.org builds are compiled using MinGW, and ship with
libraries in the A format, e.g. libmsvcrt-ruby200.dll.a.

Thankfully, we can generate a LIB from from a DLL like so:

https://adrianhenke.wordpress.com/2008/12/05/create-lib-file-from-dll/

We can also use most of the headers included in the Ruby Installer packages,
however we need a platform- and compiler-specific `ruby/config.h` for our
version of Visual C++. For this, it is enough to start compilation of Ruby on
Windows from a Visual C++ command prompt, like this:

http://blog.cyplo.net/2011/01/01/compiling-ruby-1-9-2-windows/

So for a new version of Ruby 2.x, these are the steps to add support in Gosu:

  * Download and extract a Ruby Installer package.
  * Run the steps outlined in the article above on the DLL file in `/bin`.
  * Copy the resulting LIB file and the contents of `/include` into this folder.
  * Download and extract the matching source tarball from ruby-lang.org.
  * In the `/win32` folder, run `configure.bat && nmake`. Even if compilation
    fails, there should be a `/.ext` folder in `/win32` that includes the
	compiler-specific `ruby.h` header. Copy this header into `/include` as well.
  * Create a new build configuration in `RubyGosu.vcproj` called "Release
    (Ruby 2.x)". Adjust the include paths and linker input file to match
	the new LIB file and `/include` folder. Also adjust the output path.

To make it easier to compile Gosu on a fresh system, both header and LIB files
(either generated or downloaded from ruby-lang.org) have been organised in this
folder. The copyright for all these files remains with the Ruby team and Ruby
Installer contributors.
