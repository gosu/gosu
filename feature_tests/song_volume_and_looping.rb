# Require cutting-edge development Gosu for testing.
$LOAD_PATH << '../lib'
require 'gosu'

# This kind of turned into a stress-test for Song too.

class Test < Gosu::Window
  def initialize 
    # increase update_interval to simulate very low framerate (Song will skip)
    super(900, 200, false, 30)
    
    @songs = []
    
    # Taken from Wikipedia, public domain
    # (Also, best song in the world)
    @songs << Gosu::Song.new("media/JingleBells.ogg")
    
    @songs << Gosu::Song.new("audio_formats/wav_16bit_pcm.wav")
    
    # Jingle Bells converted to MP3
    @songs << Gosu::Song.new("media/JingleBells.mp3") rescue nil
    
    # Green.it, taken from http://www.arachnosoft.com/main/
    # Copyright Maxime Abbey
    @songs << Gosu::Song.new("audio_formats/impulse_tracker.it") rescue nil
    
    # inferno.mid
    @songs << Gosu::Song.new("audio_formats/general_midi.mid") rescue nil
    
    @song = @songs.first
    @song.play(true)
    
    @another_sample = Gosu::Sample.new(self, "audio_formats/wav_16bit_pcm.wav")
  end
  
  def update
    @song.volume -= 0.05 if button_down? Gosu::KbDown
    @song.volume += 0.05 if button_down? Gosu::KbUp
    song_states = @songs.map { |s| s.paused? ? 'paused' : (s.playing? ? 'playing' : 'idle') }
    self.caption = "Vol: #{(@song.volume * 100).to_i}, " +
                   "Idx: #{@songs.index Gosu::Song.current_song}, " +
                   "paused/playing: #{song_states.inspect}"
  end
  
  def button_down(id)
    sleep 3 if button_id_to_char(id) == 's'
    
    @another_sample.play if button_id_to_char(id) == 'a'
    
    @song.play(true) if button_id_to_char(id) == 'l'
    @song.play(false) if id == Gosu::KbSpace
    @song.pause if button_id_to_char(id) == 'p'
    @song.stop if id == Gosu::KbBackspace
    ch = button_id_to_char(id)
    if ('1'..'9').include? ch then
      @song = @songs[ch.to_i - 1] unless ch.to_i > @songs.size
    end
  end
end

Test.new.show
