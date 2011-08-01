$LOAD_PATH << '../lib'
require 'gosu'

class Test < Gosu::Window
  def initialize 
    super(640, 480, false)
    @sample = Gosu::Sample.new("media/Sample.wav")
  end
  
  def needs_cursor?; true; end
  
  def update
    @instance = nil if @instance and not @instance.playing?
    self.caption = "Playing: #{not @instance.nil?}"
  end
  
  def button_down(id)
    case id
    when Gosu::KbEscape then close
    when Gosu::MsLeft then
      @instance = @sample.play_pan mouse_x/320 - 1, 1, mouse_y/480 + 0.5
    end
  end
end

Test.new.show
