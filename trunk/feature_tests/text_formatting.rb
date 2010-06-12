require '../lib/gosu'

class Test < Gosu::Window
  def initialize
    super(800, 600, false)

    @font = Gosu::Font.new(self, Gosu::default_font_name, 50)
  end
  
  def draw
    @font.draw("Hal<b>lo We<i>lt<c=ffff00>yell</i>ow</c>white", 10, 10, 0)
  end
  
  def button_down(id)
    close if id == Gosu::KbEscape
  end
end

Test.new.show
