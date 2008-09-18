#!/usr/bin/env ruby
# Bogus extconf.rb to bridge rubygems to autoconf.
# rubygems will call "make install" on the resulting makefile.

require 'mkmf'

Dir.chdir File.dirname($0)

ruby_command = "ruby#{RUBY_VERSION[0..2]}"

exit 1 if not system("env GOSU_MAKEFILE_FOR_GEM=1 GOSU_RUBY_COMMAND=#{ruby_command} sh configure")

exit 0
