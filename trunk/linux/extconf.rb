#!/usr/bin/env ruby

if defined? RUBY_PLATFORM and (RUBY_PLATFORM['-win32'] or RUBY_PLATFORM['win32-'] or RUBY_PLATFORM['-mingw32']) then
  puts 'This gem is not meant to be installed on Windows. If you see this message, gem made a mistake.'
  puts 'On Windows, please use:'
  puts 'gem install gosu --platform=mswin32'
  exit 1
end

puts 'The Gosu gem requires some libraries to be installed system-wide.'
puts 'See the following site for a list:'
puts 'http://code.google.com/p/gosu/wiki/GettingStartedOnLinux'
puts

require 'mkmf'

dir_config("gosu")

create_makefile("gosu")

