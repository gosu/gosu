require '../gosu'

class GameWindow < Gosu::Window
  def initialize
    super(800, 600, false)
      
    self.caption = "Huhu, 地下鉄で行きます"
    @font = Gosu::Font.new(self, "Monaco", 15)
    @sys_text = Gosu::Image.from_text(self, "Huhu, 地下 hihi 鉄で行 haha きます", "Helvetica", 15, 0, 200, :left)
    @loc_text = Gosu::Image.from_text(self, "Huhu, 地下鉄で行きます", "./Vera.ttf", 30, 0, 200, :left)
  end

  def draw
    draw_line(100, 300, 0xffffffff, 300, 300, 0xffffffff, 255)
    draw_line(300, 304, 0xffffffff, 100, 304, 0xffffffff, 255)
    @sys_text.draw(100, 100, 0)
    @loc_text.draw(400, 100, 0)
    @font.draw(('A'..'Z').to_a.join + ('a'..'z').to_a.join, 10, 10, 0)
    @font.draw_rel("Alfons läuft durch Bayern und fühlt sich pudelwohl dabei! Na sowas.", 800, 600, 0, 0.5, 0.5)
  end
  
  def button_down(id)
    close if id == Gosu::KbEscape
  end
end

window = GameWindow.new
window.show
