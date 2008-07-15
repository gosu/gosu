require '../gosu'

class Test < Gosu::Window
  def initialize
    super(800, 600, false)
      
    self.caption = "Huhu, 地下鉄で行きます"
    @font = Gosu::Font.new(self, "Monaco", 15)
  end

  def draw
    draw_quad(0, 0, 0x80eaf57b, 800, 0, 0x805437fe, 0, 600, 0x80ba932e, 800, 600, 0x8023bcc1, 255)
    clip_to mouse_x.to_i, mouse_y.to_i, 600, 400 do
      draw_quad(0, 0, 0xffeaf57b, 800, 0, 0xff5437fe, 0, 600, 0xffba932e, 800, 600, 0xff23bcc1, 1)
      @font.draw(('A'..'Z').to_a.join + ('a'..'z').to_a.join, 10, 10, 1)
      @font.draw_rel("Alfons läuft durch Bayern und fühlt sich pudelwohl dabei! Na sowas.", 800, 600, 1, 1, 1)
    end
    draw_quad(0, 0, 0x80eaf57b, 800, 0, 0x805437fe, 0, 600, 0x80ba932e, 800, 600, 0x8023bcc1, 0)
  end
  
  def button_down(id)
    close if id == Gosu::KbEscape
  end
end

Test.new.show
