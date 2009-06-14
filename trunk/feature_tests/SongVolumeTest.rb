# Require cutting-edge development Gosu for testing.
require '../lib/gosu'

# This kind of turned into a stress-test for song too.

class Test < Gosu::Window
  def initialize 
    # increase update_interval to simulate very low framerate (Song will skip)
    super(640, 480, false, 30)
    # Taken from Wikipedia, public domain
    # (Also, best song in the world)
    ogg = Gosu::Song.new(self, "media/Jingle_Bells.ogg")
    # Taken from http://www.arachnosoft.com/main/
    # Copyright Maxime Abbey
    #it = Gosu::Song.new(self, "media/Green.it")
    # MIDI not yet supported everywhere :( Should be implemented
    # using OS APIs where missing.
    #mid = it #Gosu::Song.new(self, "media/Inferno.mid")
    # Looping WAV
    #wav = Gosu::Song.new(self, "media/Loop.wav")
    @songs = [ogg]#, it, mid, wav]
    @song = @songs[0]
    @song.play(true)
    
    @another_sample = Gosu::Sample.new(self, "media/Sample.wav")
  end

  def update
    @song.volume -= 0.05 if button_down? Gosu::KbDown
    @song.volume += 0.05 if button_down? Gosu::KbUp
    song_index = @songs.index(Gosu::Song.current_song) || "none"
    #songs_paused = @songs.map { |s| s.paused? }
    #self.caption = "Volume: #{(@song.volume * 100).to_i}, Song: #{song_index}, Paused: #{songs_paused.inspect}"
  end
  
  def button_down(id)
    sleep 3 if button_id_to_char(id) == 's'
    
    @another_sample.play if button_id_to_char(id) == 'a'
    
    @song.play(true) if button_id_to_char(id) == 'l'
    @song.play(false) if id == Gosu::KbSpace
    @song.pause if button_id_to_char(id) == 'p'
    @song.stop if id == Gosu::KbBackspace
    @song = @songs[0] if button_id_to_char(id) == '1'
    @song = @songs[1] if button_id_to_char(id) == '2'
    @song = @songs[2] if button_id_to_char(id) == '3'
    @song = @songs[3] if button_id_to_char(id) == '4'
  end
end

Test.new.show
