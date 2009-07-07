# Require cutting-edge development Gosu for testing.
require '../lib/gosu'
require 'fps_counter'

class Test < Gosu::Window
  def draw_grid
    160.times do |x|
      120.times do |y|
        @image.draw x * 3, y * 3, 0, 0.01, 0.01
      end
    end
  end
  
  def initialize 
    super(640, 480, false)
    
    @image = Gosu::Image.new(self, "media/SquareTexture.png", true)
    @macro = record { draw_grid }
  end
  
  def button_down(id)
    close if id == Gosu::KbEscape
  end
  
  def draw
    @fps_counter ||= FPSCounter.new
    @fps_counter.register_tick
    
    use_macro = !button_down?(Gosu::KbTab)
    
    self.caption = "#{@fps_counter.fps} FPS, Macro: #{use_macro}"
    if use_macro then
      @macro.draw 10, 10, 0
    else
      draw_grid
    end
  end
end

Test.new.show
