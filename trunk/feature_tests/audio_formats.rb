# Require cutting-edge development Gosu for testing.
require '../lib/gosu'

w = Gosu::Window.new(100, 100, false)
Dir.chdir 'audio_formats'
Dir['*mp3*'].each do |format|
  puts "Testing #{format}:"
  begin
    si = Gosu::Sample.new(w, format).play
    sleep 1
    si.stop
    puts " X Sample"
  rescue
    puts "   Sample (#$!)"
  end
  begin
    Gosu::Song.new(w, format).play
    sleep 1
    Gosu::Song::current_song.stop if Gosu::Song::current_song
    puts " X Song"
  rescue
    puts "   Song (#$!)"
  end
end
