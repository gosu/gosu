# Require cutting-edge development Gosu for testing.
require '../lib/gosu'

w = Gosu::Window.new(100, 100, false)
Dir.chdir 'audio_formats'
FORMATS_TO_TRY = Dir['*'].sort

puts "|| Platform || #{FORMATS_TO_TRY.map { |fn| fn[/[^\.]*/].gsub('_', ' ') }.join(' || ')} ||"

my_platform = case RUBY_PLATFORM
  when /darwin/ then 'OS X'
  when /win/ then 'Windows'
  else 'Linux'
end

# Sample

print "|| #{my_platform} (Gosu::Sample)"
FORMATS_TO_TRY.each do |format|
  print " || "
  begin
    si = Gosu::Sample.new(w, format).play
    sleep 1
    si.stop
    print "Yes"
  rescue
    # Do nothing
  end
end
puts " ||"

# Song

print "|| #{my_platform} (Gosu::Song) "
FORMATS_TO_TRY.each do |format|
  print " || "
  begin
    Gosu::Song.new(w, format).play
    sleep 1
    Gosu::Song::current_song.stop if Gosu::Song::current_song
    print "Yes"
  rescue
    # Do nothing
  end
end
puts " ||"
