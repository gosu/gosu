require '../lib/gosu'

class Test < Gosu::Window
  def initialize 
    super(640, 480, false)
    @sample = Gosu::Sample.new(self, "media/Jingle_Bells.ogg")
    @instance = @sample.play
  end

  def update
    @instance = @sample.play if not @instance.playing?
    @instance.pan = Gosu::random(-1, 1) if button_down?(Gosu::KbTab)
  end
  
  def button_down(id)
    close if id == Gosu::KbEscape
    @instance.stop if id == Gosu::KbSpace
    @instance.speed = 0.5 if id == Gosu::KbLeft
  end
end

Test.new.show
