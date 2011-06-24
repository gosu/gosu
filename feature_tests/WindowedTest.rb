$LOAD_PATH << '../lib'
require 'gosu'

W, H = 4000, 2000

class Test < Gosu::Window
  def initialize
    super W, H, false
    @wp = Gosu::Image.new self, "media/Wallpaper.png", true
    @cursor = Gosu::Image.new self, "media/Cursor.png", false
    self.caption = "I am #{width} x #{height}"
  end
  
  def draw
    @wp.draw W - @wp.width, H - @wp.height, 0
    @cursor.draw mouse_x, mouse_y, 0
  end

  def button_down(id)
    if id == Gosu::Button::KbEscape
      close
    end
  end
end

Test.new.show
