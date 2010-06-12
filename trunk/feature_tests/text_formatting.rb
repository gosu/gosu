require '../lib/gosu'

class Test < Gosu::Window
  def initialize
    super(800, 600, false)

    @font = Gosu::Font.new(self, Gosu::default_font_name, 50)
  end
  
  def draw
    @font.draw("</u>H<u>al<b>lo We<i>lt<c=ffff00>y<u>ell</i>ow</c>w</u>hite", 10, 10, 0)
  end
  
  def button_down(id)
    close if id == Gosu::KbEscape
  end
end

Test.new.show
