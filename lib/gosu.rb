require 'rbconfig'

if RUBY_PLATFORM =~ /mswin$|mingw32|mingw64|win32\-|\-win32/ then
  binary_path = File.dirname(__FILE__)
  # 64-bit builds of Windows use "x64-mingw32" as RUBY_PLATFORM
  binary_path += '64' if RUBY_PLATFORM =~ /^x64-/
  
  # Add this gem to the PATH on Windows so that bundled DLLs can be found.
  ENV['PATH'] = "#{binary_path};#{ENV['PATH']}"
  
  # Add the correct directory 
  RUBY_VERSION =~ /(\d+.\d+)/
  $LOAD_PATH.unshift "#{binary_path}/#{$1}"
end

require "gosu.#{RbConfig::CONFIG['DLEXT']}"

require "gosu/swig_patches"
require "gosu/patches"
