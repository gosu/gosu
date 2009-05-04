#!/usr/bin/env ruby

output = `nm '../RubyGosu\ App.app/Contents/MacOS/RubyGosu\ App'`
exports = output.grep /\ (_rb_|_ruby_|_st_|_SIGNED)/
exports.map! { |line| line.split(' ').last }
exports.reject! { |symbol| symbol.include? '.' }
puts exports
