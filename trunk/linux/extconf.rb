#!/usr/bin/env ruby
# Bogus extconf.rb to bridge rubygems to autoconf.
# rubygems will call "make install" on the resulting makefile.

if defined? RUBY_PLATFORM and (RUBY_PLATFORM[0..4] == 'win32' or RUBY_PLATFORM['-mingw32']) then
  puts 'This gem is not meant to be installed on Windows.'
  puts 'On Windows, please use:'
  puts 'gem install gosu --platform=mswin32'
  exit 1
end

puts 'The Gosu gem requires some libraries to be installed system-wide.'
puts 'See the following site for a list:'
puts 'http://code.google.com/p/gosu/wiki/GettingStartedOnLinux'
puts

require 'rbconfig'

Dir.chdir File.dirname($0)

ruby_command = File.join(Config::CONFIG["bindir"], Config::CONFIG["RUBY_INSTALL_NAME"])

exit 1 if not system("env GOSU_MAKEFILE_FOR_GEM=1 GOSU_RUBY_COMMAND=#{ruby_command} sh configure")
