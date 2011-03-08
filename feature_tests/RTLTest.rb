require '../lib/gosu'

class Test < Gosu::Window
  def initialize
    super(800, 600, false)

    @font = Gosu::Font.new(self, Gosu::default_font_name, 50)
  end
  
  def draw
    draw_quad 0,0,0xffff0000,800,0,0xffff0000,0,600,0xffff0000,800,600,0xffff0000,0
    @font.draw("Erstens‮Zweitens‭Drittens", 10, 10, 0)
    @font.draw("‮Alleinstehend :(", 10, 50, 0)
    @font.draw("שלום העולם!", 10, 90, 0)
  end
  
  def button_down(id)
    close if id == Gosu::KbEscape
  end
end

Test.new.show
