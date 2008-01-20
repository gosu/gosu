require '../gosu'

class GameWindow < Gosu::Window
  def initialize
    super(640, 480, true)
  end
  
  def draw
    draw_quad(0, 0, 0xffff0000, 640, 0, 0xffff0000,
      0, 480, 0xffffff00, 640, 480, 0xffffff00, 0)
  end

  def button_down(id)
    if id == Gosu::Button::KbEscape
      close
    end
  end
end

window = GameWindow.new
window.show
