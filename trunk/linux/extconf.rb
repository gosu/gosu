#!/usr/bin/env ruby
# Bogus extconf.rb to bridge rubygems to autoconf.
# rubygems will call "make install" on the resulting makefile.

puts 'The Gosu gem requires some libraries to be installed system-wide.'
puts 'See the following site for a list:'
puts 'http://code.google.com/p/gosu/wiki/GettingStartedOnLinux'
puts

require 'mkmf'

Dir.chdir File.dirname($0)

ruby_command = "ruby#{RUBY_VERSION[0..2]}"

exit 1 if not system("env GOSU_MAKEFILE_FOR_GEM=1 GOSU_RUBY_COMMAND=#{ruby_command} sh configure")

exit 0

