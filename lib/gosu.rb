require 'rbconfig'

if RUBY_PLATFORM =~ /mswin$|mingw32|win32-|-win32/ then
  # 64-bit builds of Windows use "x64-mingw32" as RUBY_PLATFORM
  suffix = '64' if RUBY_PLATFORM =~ /^x64-/
  
  # Add this gem to the PATH on Windows so that DLLs can be found.
  ENV['PATH'] = "#{File.dirname(__FILE__)}#{suffix};#{ENV['PATH']}"
end

begin
  # If possible, use a precompiled binary.
  RUBY_VERSION =~ /(\d+.\d+)/
  require "../lib#{suffix}/#{$1}/gosu.#{RbConfig::CONFIG['DLEXT']}"
rescue LoadError
  require "gosu.#{RbConfig::CONFIG['DLEXT']}"
end

require "gosu/swig_patches"
require "gosu/patches"
