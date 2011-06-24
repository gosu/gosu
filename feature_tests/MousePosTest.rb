$LOAD_PATH << '../lib'
require 'gosu'

class Test < Gosu::Window
  def initialize
    super(480, 320, false)
    @cursor = Gosu::Image.new(self, "media/Cursor.png", false)
  end
  
  def button_down id
    if id == Gosu::KbEscape then
      close
    else
      if rand < 0.5 then
       set_mouse_position mouse_x / 2, mouse_y
      else
       self.mouse_x /= 2
      end
    end
  end
  
  def draw
    @cursor.draw mouse_x, mouse_y, 0
  end
end

Test.new.show
