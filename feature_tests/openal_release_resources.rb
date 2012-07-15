$LOAD_PATH << '../lib'
require 'gosu'

$stderr.sync = true
$stdout.sync = true

at_exit { puts "Ruby at_exit" }

Gosu::Sample.new("C:\\Windows\\Media\\Windows Ringout.wav").play
sleep 1
