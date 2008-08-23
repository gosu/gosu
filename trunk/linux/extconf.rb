#!/usr/bin/env ruby
# bogus extconf.rb to bridge rubygems to autoconf

require 'mkmf'

Dir.chdir(File.dirname($0))
if system('autoconf')
  if system('./configure')
    exit 0
  end
end

exit 1
