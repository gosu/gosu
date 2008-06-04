require '../gosu'

class Test < Gosu::Window
  def initialize
    super(640, 480, true)
  end
  
  def draw
    draw_quad(0, 0, 0xffff0000, 630, 10, 0xffff0000,
      10, 470, 0xffffff00, 640, 480, 0xffffff00, 0)
  end

  def button_down(id)
    if id == Gosu::Button::KbEscape
      close
    end
  end
end

Test.new.show
