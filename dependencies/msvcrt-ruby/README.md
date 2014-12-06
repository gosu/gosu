# Ruby MSVCRT LIB Files

Gosu (the C++ library) and its Ruby extension are compiled using Microsoft
Visual C++, for which we need both headers and LIB files to link against.

Binaries from ruby-lang.org (up to Ruby 1.9.x) were compiled using MSVC and have
included libraries in the LIB format that Gosu could be linked against. However,
all current rubyinstaller.org builds are compiled using MinGW, and ship with
libraries in the A format, e.g. libmsvcrt-ruby200.dll.a.

Thankfully, we can generate a LIB from file a DLL like so:

https://adrianhenke.wordpress.com/2008/12/05/create-lib-file-from-dll/

To generate matching headers, it is enough to start compilation of Ruby on
Windows from a Visual C++ command prompt.

To make it easier to compile Gosu on a fresh system, both header and LIB files
(either generated or downloaded from ruby-lang.org) have been organised in this
folder. The copyright for all these files remains with the Ruby team and Ruby
Installer contributors.
