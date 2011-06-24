$LOAD_PATH << '../lib'
require 'gosu'

class GameWindow < Gosu::Window
  def initialize
    super(640, 480, true, 1)
    @font = Gosu::Font.new(self, Gosu::default_font_name, 20)
  end

  def draw
    color = Gosu::Color.from_hsv(Gosu::milliseconds / 20, 1, 0.5 + 0.5 * Math.sin(Gosu::milliseconds / 100.0))
    draw_quad 0, 0, color, 640, 0, color, 0, 480, color, 640, 480, color, 0
    @font.draw "#{Gosu::fps} FPS", 10, 10, 0
  end

  def button_down(id)
    if id == Gosu::Button::KbEscape
      close
    end
  end
end

window = GameWindow.new
window.show
