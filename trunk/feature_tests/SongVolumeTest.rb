# Require cutting-edge development Gosu for testing.
require '../gosu'

class Test < Gosu::Window
  def initialize 
    super(640, 480, false)
    # Taken from Wikipedia, public domain
    ogg = Gosu::Song.new(self, "media/Jingle_Bells.ogg")
    # Taken from http://www.arachnosoft.com/main/
    # Copyright Maxime Abbey
    it = Gosu::Song.new(self, "media/Green.it")
    # MIDI not yet supported :( Should be implemented
    # using OS APIs.
    mid = it ##Gosu::Song.new(self, "media/Inferno.mid")
    @songs = [ogg, it, mid]
    @song = @songs[0]
    @song.play
  end

  def update
    @song.volume -= 0.05 if button_down? Gosu::KbDown
    @song.volume += 0.05 if button_down? Gosu::KbUp
    self.caption = "Volume: #{(@song.volume * 100).to_i}"
  end
  
  def button_down(id)
    @song.play if id == Gosu::KbSpace
    @song.stop if id == Gosu::KbBackspace
    @song = @songs[0] if button_id_to_char(id) == '1'
    @song = @songs[1] if button_id_to_char(id) == '2'
    @song = @songs[2] if button_id_to_char(id) == '3'
  end
end

Test.new.show
