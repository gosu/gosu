# Require cutting-edge development Gosu for testing.
$LOAD_PATH << '../lib'
require 'gosu'

class RecordPerformanceTest < Gosu::Window
  def grid_x
    20 + Math.sin(Gosu::milliseconds / 200.0) * 10
  end
  
  def grid_y
    20 + Math.cos(Gosu::milliseconds / 200.0) * 10
  end
  
  def draw_grid base_x, base_y
    400.times do |x|
      300.times do |y|
        # This will split the VertexArray into single operations - harrgggghh!!
        #@images[x % 2].draw base_x + x * 2, base_y + y * 2, 0, 0.01, 0.01, 0x80ffffff
        @images[0].draw base_x + x * 2, base_y + y * 2, 0, 0.01, 0.01, 0x80ffffff
      end
    end
  end
  
  def initialize
    super(900, 700, false)
    
    @images = %w(Wallpaper.png SquareTexture.png).map do |filename|
      Gosu::Image.new(self, "media/#{filename}", true)
    end
    @macro = record(1000, 1000) { draw_grid 0, 0 }
  end
  
  def button_down(id)
    close if id == Gosu::KbEscape
  end
  
  def draw
    use_macro = !button_down?(Gosu::KbTab)
    
    self.caption = "#{Gosu::fps} FPS, Macro? #{use_macro}"
    if use_macro then
      zoom = button_down?(char_to_button_id('+')) ? 100.0 : 1.0
      @macro.draw grid_x, grid_y, 0, zoom, zoom
    else
      draw_grid grid_x, grid_y
    end
  end
end

RecordPerformanceTest.new.show
