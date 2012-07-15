$LOAD_PATH << '../lib'
require 'gosu'

s = Gosu::Sample.new("C:\\Windows\\Media\\Windows Ringout.wav")
s.play
s2 = Gosu::Song.new("C:\\Windows\\Media\\tada.wav")
s2.play(true)
sleep 0.3
