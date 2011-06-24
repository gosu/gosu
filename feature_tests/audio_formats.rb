# Encoding: UTF-8

# Require cutting-edge development Gosu for testing.
$LOAD_PATH << '../lib'
require 'gosu'

Dir.chdir 'audio_formats'
FORMATS_TO_TRY = Dir['*'].sort
FORMAT_NAMES = FORMATS_TO_TRY.map { |fn| fn[/[^\.]*/].gsub('_', ' ') }

def test_sample format
  si = Gosu::Sample.new(format).play
  sleep 1
  si.stop
  '&#x2713;'
rescue
  ''
end

def test_song format
  Gosu::Song.new(format).play
  sleep 1
  Gosu::Song::current_song.stop if Gosu::Song::current_song
  '&#x2713;'
rescue
  ''
end

class String
  def widen(length)
    "%#{length}s" % self
  end
end

platform = case RUBY_PLATFORM
  when /darwin/ then 'OS X'
  when /win/, /mingw/ then 'Windows'
  else 'Linux'
end

format_length = FORMAT_NAMES.max_by(&:length).length
sample_length = "Sample (#{platform})".length
song_length   = "Song (#{platform})".length

puts "#{'Format'.widen(format_length)}|Sample (#{platform})|Song (#{platform})"
puts "#{'-' * format_length}|#{'-' * sample_length}|#{'-' * song_length}"

FORMATS_TO_TRY.each_with_index do |format, i|
  print FORMAT_NAMES[i].widen(format_length)
  print '|'
  print test_sample(format).widen(sample_length)
  print '|'
  print test_song(format).widen(song_length)
  puts
end
