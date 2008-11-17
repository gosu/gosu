require '../lib/gosu'

class Test < Gosu::Window
  def initialize
    super(800, 600, false)

    @font = Gosu::Font.new(self, Gosu::default_font_name, 50)
  end
  
  def draw
    @font.draw("Erstens‮Zweitens‭Drittens", 10, 10, 0)
    @font.draw("‮Alleinstehend :(", 10, 50, 0)
    @font.draw("שלום העולם!", 10, 90, 0)
  end
  
  def button_down(id)
    close if id == Gosu::KbEscape
  end
end

Test.new.show
