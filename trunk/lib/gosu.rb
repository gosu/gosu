require 'rbconfig'

begin
  if defined? RUBY_VERSION and RUBY_VERSION[0..2] == '1.9' then
    version = '1_9'
  else
    version = '1_8'
  end
  require "#{File.dirname(__FILE__)}/gosu.for_#{version}.#{Config::CONFIG['DLEXT']}"
  require "#{File.dirname(__FILE__)}/gosu/swig_patches.rb"
rescue LoadError => e
  require "#{File.dirname(__FILE__)}/gosu.custom.#{Config::CONFIG['DLEXT']}"
  require "#{File.dirname(__FILE__)}/gosu/swig_patches.rb"
end

require "#{File.dirname(__FILE__)}/gosu/patches.rb"
