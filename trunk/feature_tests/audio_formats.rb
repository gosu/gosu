# Require cutting-edge development Gosu for testing.
require '../lib/gosu'

w = Gosu::Window.new(100, 100, false)
Dir.chdir 'audio_formats'
Dir['*'].each do |format|
  puts "Testing #{format}:"
  begin
    Gosu::Sample.new(w, format).play
    sleep 1
    puts " X Sample"
  rescue
    puts "   Sample (#$!)"
  end
  begin
    Gosu::Song.new(w, format).play
    sleep 1
    puts " X Song"
  rescue
    puts "   Song (#$!)"
  end
end
