require '../gosu'

class GameWindow < Gosu::Window
  def initialize
    super(800, 600, true)
      
    self.caption = "Huhu, 地下鉄で行きます"
    @sys_text = Gosu::Image.from_text(self, "Huhu, 地下 hihi 鉄で行 haha きます", "Comic Sans MS", 10, 0, 200, :left)
    @loc_text = Gosu::Image.from_text(self, "Huhu, 地下鉄で行きます", "./Vera.ttf", 10, 0, 200, :left)
  end

  def draw
    draw_line(100, 300, 0xffffffff, 300, 300, 0xffffffff, 255)
    draw_line(300, 304, 0xffffffff, 100, 304, 0xffffffff, 255)
    @sys_text.draw(100, 100, 0)
    @loc_text.draw(400, 100, 0)
  end
  
  def button_down(id)
    close if id == Gosu::KbEscape
  end
end

window = GameWindow.new
window.show
