require '../lib/gosu'

class Test < Gosu::Window
  def initialize 
    super(640, 480, false)
    @sample = Gosu::Sample.new(self, "media/Sample.wav")
  end
  
  def update
    @instance = nil if @instance and not @instance.playing?
    self.caption = "Playing: #{not @instance.nil?}"
  end
  
  def button_down(id)
    case id
    when Gosu::KbEscape then close
    when Gosu::KbSpace then @instance = @sample.play
    end
  end
end

Test.new.show
