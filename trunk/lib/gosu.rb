require 'rbconfig'

begin
  if RUBY_VERSION and RUBY_VERSION[0..2] == '1.9' then
    require "#{File.dirname(__FILE__)}/gosu.for_1_9.#{Config::CONFIG['DLEXT']}"
  else
    require "#{File.dirname(__FILE__)}/gosu.for_1_8.#{Config::CONFIG['DLEXT']}"
  end
rescue LoadError => e
  require "#{File.dirname(__FILE__)}/gosu.#{Config::CONFIG['DLEXT']}"
end
