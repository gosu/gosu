require '../gosu'

class Test < Gosu::Window
  def initialize 
    super(640, 480, false)
    @sample = Gosu::Sample.new(self, "Sample.wav")
    @instance = @sample.play
  end

  def update
    @instance = @sample.play if not @instance.playing?
    #@instance.pan = Gosu::random(-1, 1)
  end
  
  def button_down(id)
    close if id == Gosu::KbEscape
    @instance.stop if id == Gosu::MsLeft
    @instance.speed = 0.5 if id == Gosu::MsRight
    p id
  end
end

Test.new.show
