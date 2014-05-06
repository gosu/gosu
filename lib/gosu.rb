require 'rbconfig'

if RUBY_PLATFORM =~ /mswin|mingw32|mingw64|win32\-|\-win32/ then
  # Add this gem to the PATH on Windows so that DLLs can be found
  ENV['PATH'] = "#{File.dirname(__FILE__)};#{ENV['PATH']}"
end

begin
  RUBY_VERSION =~ /(\d+.\d+)/
  require "#{$1}/gosu.#{RbConfig::CONFIG['DLEXT']}"
rescue LoadError
  require "gosu.#{RbConfig::CONFIG['DLEXT']}"
end

require "gosu/swig_patches"
require "gosu/patches"
