require '../lib/gosu'

class Test < Gosu::Window
  def initialize
    super 4000, 2000, false
    @wp = Gosu::Image.new self, "media/Wallpaper.png", true
    @cursor = Gosu::Image.new self, "media/Cursor.png", false
  end
  
  def draw
    @wp.draw 4000 - @wp.width, 2000 - @wp.height, 0
    @cursor.draw mouse_x, mouse_y, 0
  end

  def button_down(id)
    if id == Gosu::Button::KbEscape
      close
    end
  end
end

Test.new.show
