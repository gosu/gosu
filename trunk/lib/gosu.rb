require 'rbconfig'

begin
  if defined? NSObject then
    require "#{File.dirname(__FILE__)}/gosu.for_macruby.bundle"
    require "#{File.dirname(__FILE__)}/hotgosu.rb"
  elsif defined? RUBY_VERSION and RUBY_VERSION[0..2] == '1.9' then
    require "#{File.dirname(__FILE__)}/gosu.for_1_9.#{Config::CONFIG['DLEXT']}"
    require "#{File.dirname(__FILE__)}/gosu/swig_patches.rb"
  else
    require "#{File.dirname(__FILE__)}/gosu.for_1_8.#{Config::CONFIG['DLEXT']}"
    require "#{File.dirname(__FILE__)}/gosu/swig_patches.rb"
  end
rescue LoadError => e
  require "#{File.dirname(__FILE__)}/gosu.custom.#{Config::CONFIG['DLEXT']}"
  require "#{File.dirname(__FILE__)}/gosu/swig_patches.rb"
end

require "#{File.dirname(__FILE__)}/gosu/patches.rb"
