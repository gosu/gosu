require 'gosu'

class GameWindow < Gosu::Window
  def initialize
    super(640, 480, false, 1)
      
    self.caption = "Huhu, 地下鉄で行きます"
    @sys_text = Gosu::Image.from_text(self, "Huhu, 地下 hihi 鉄で行 haha きます", "Comic Sans MS", 10, 0, 200, :left)
    @loc_text = Gosu::Image.from_text(self, "Huhu, 地下鉄で行きます", "./Vera.ttf", 10, 0, 200, :left)
  end

  def draw
    @sys_text.draw(100, 100, 0)
    @loc_text.draw(400, 100, 0)
  end
end

window = GameWindow.new
window.show
