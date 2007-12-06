require 'gosu'

class GameWindow < Gosu::Window
  def initialize
    super(640, 480, false, 1)
        
    @sys_text = Gosu::Image.from_text(self, "Hallo Wält!", "Zapfino", 100, 0, 200, :left)
    @loc_text = Gosu::Image.from_text(self, "Hallo Wält!", "./Vera.ttf", 100, 0, 200, :left)
  end

  def draw
    @sys_text.draw(100, 100, 0)
    @loc_text.draw(400, 100, 0)
  end
end

window = GameWindow.new
window.show
