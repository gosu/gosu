require 'rbconfig'

WINDOWS_HINTS = %w(-win32 win32- mswin mingw32)

if defined? RUBY_PLATFORM and WINDOWS_HINTS.any? { |hint| RUBY_PLATFORM.include? hint } then
  ENV['PATH'] = "#{File.dirname(__FILE__)};#{ENV['PATH']}"
end

if File.exist? "#{File.dirname(__FILE__)}/gosu.#{Config::CONFIG['DLEXT']}"
  require "gosu.#{Config::CONFIG['DLEXT']}"
elsif defined? RUBY_VERSION and RUBY_VERSION.split('.')[1].to_i > 8 then
  require "gosu.for_1_9.#{Config::CONFIG['DLEXT']}"
else
  require "gosu.for_1_8.#{Config::CONFIG['DLEXT']}"
end

require "gosu/swig_patches"
require "gosu/patches"
