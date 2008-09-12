#!/usr/bin/env ruby
# Bogus extconf.rb to bridge rubygems to autoconf.
# rubygems will call "make install" on the resulting makefile.

require 'mkmf'

Dir.chdir File.dirname($0)

exit 1 if not system('env GOSU_MAKEFILE_FOR_GEM=1 sh configure')

exit 0
