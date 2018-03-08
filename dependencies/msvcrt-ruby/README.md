# Ruby Header/Library Files for Microsoft Visual C++

## Background

Gosu (the C++ library) and its Ruby extension are compiled using Microsoft
Visual C++. This requires header files for Ruby's C interface as well as a
library in the `.lib` format to link against.

All current [Ruby Installer](http://rubyinstaller.org/) builds are
compiled using MinGW, and only ship with libraries in the `.a` format, e.g.
`libmsvcrt-ruby200.dll.a`, which is not compatible with Visual C++.

Luckily, the Ruby Installer packages include a DLL that we can use to generate
a `.lib` file like so:

https://adrianhenke.wordpress.com/2008/12/05/create-lib-file-from-dll/

We can also use most of the headers included in the Ruby Installer packages.
However we need a platform- and compiler-specific `ruby/config.h` for our
version of Visual C++.

For this, it is enough to start compilation of Ruby on Windows from a Visual C++
command prompt, like this:

[http://blog.cyplo.net/2011/01/01/compiling-ruby-1-9-2-windows/](http://web.archive.org/web/20140623200126/http://blog.cyplo.net/2011/01/01/compiling-ruby-1-9-2-windows/)

Even if compilation fails, the correct `ruby/config.h` file will be written.

## Step by Step

For example, to add headers and libraries for Ruby 2.5, follow these steps:

  * Download and install the Ruby Installer package.
  * Run the steps outlined in the first article above on the DLL file in `bin`:
    * Launch the Developer Command Prompt, NOT the one that starts with "MSBuild".
    * `cd` into `C:\Ruby25[-64]\bin`.
    * `dumpbin /exports [x64-]msvcrt-rubyxy.dll > [x64-]msvcrt-rubyxy.def`.
    * Open the `.def` file using Notepad++.
    * Delete all lines above and below the list of exported function names.
    * Also delete the lines at the top that start with `.refptr`.
    * Delete all columns before the function names (alt-selecting a rectangle
      helps).
    * **Important:** Insert the line "EXPORTS" at the top of the file.
    * Save the `.def` file, then hop back into the command prompt.
    * `lib /machine:[x64|x86] /def:[x64-]msvcrt-ruby250.def /out:[x64-]msvcrt-ruby250.lib`
  * Copy the resulting `.lib` file and the `include/ruby-2.5.0` directory into
    *this* directory, then clean up any `*mingw*` folders inside of the
    `ruby-2.5.0` folder since we are building for Visual C++, not MinGW.
  * Download and extract the matching source tarball from
    https://www.ruby-lang.org/en/downloads/ (`ruby-2.5.0.tar.gz` in this case).
  * In the `ruby-2.5.0` (source) directory, run `win32\configure.bat && nmake`.
    Even if compilation fails, there should be a new `.ext\include` folder that
    contains the compiler-specific `ruby/config.h` header we are looking for.
    Specifically, copy the `.ext\include\i386-mswin32_140` directory into
    `ruby-2.5.0` in this directory. These are the headers for compiling 32-bit
    C extensions.
  * To generate the header file for the x64 DLL, run
    `win32\configure.bat --target=x64-mswin64 && nmake`. This fails immediately
    for me (error C2118), but the generated header looks correct. Copy it as well.
  * Open `windows\Gosu.sln`, right-click the root element and open the
    Configuration Manager.
  * Create a new solution configuration (top left corner) called "Release (Ruby 2.4)"
    as a copy of "Release".
  * Go back to the "Release" solution configuration.
  * Create a new build configuration for `RubyGosu.vcproj` called "Release
    (Ruby 2.5)" as a copy of "Release (Ruby 2.4)".
    **Do not** create another new solution configuration (checkbox at the bottom).
  * Ensure that "Release" uses the "Release (Ruby 2.5)" configuration for both Win32
    and x64.
  * Oops, now we have created a "Release (Ruby 2.4)" configuration for the Gosu *project*
    (not *solution*); delete it. (Gosu for C++ only needs Release and Debug.)
  * Close the Configuration Manager and open the Properties page for the RubyGosu
    project.
    Adjust the include paths and linker input files for the Ruby 2.5 configuration to use
    the new `.lib` file and `include` folder instead of the ones from Ruby 2.4.
  * Also adjust the output path to be `..\lib\x.y`.
  * Double-check that you have both updated the Win32 and x64 configurations.
  * Now you should be able to batch-compile RubyGosu for all relevant platforms.

## License

To make it easier to compile Gosu on a fresh system, the resulting header and `.lib`
files for each Ruby version have been collected in this directory.

The copyright for all these files remains with the Ruby team and Ruby
Installer contributors.
