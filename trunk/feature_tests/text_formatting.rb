require '../lib/gosu'

TEST_STRING = "</u>H<u>al<b>lo &amp;&lt;b&gt; We<i>lt Ren & Stimpy Entity: &cursor; <c=ffff00>y<u>e<c=80ffff00>ll</i>ow</c></c>w</u>hite"

class Test < Gosu::Window
  def initialize
    super(800, 600, false)
    
    Gosu::register_entity "cursor", Gosu::Image.new(self, "media/Cursor.png", false)
    
    @font = Gosu::Font.new(self, Gosu::default_font_name, 50)
    
    @simple_text = Gosu::Image.from_text(self, "#{TEST_STRING}\n#{TEST_STRING}", Gosu::default_font_name, 50)

    @complex_text = Gosu::Image.from_text(self, "#{TEST_STRING} #{TEST_STRING} a b c test 123  #{TEST_STRING}" * 4, Gosu::default_font_name, 25, 5, 350, :left)
  end
  
  def draw
    # TODO: get rid of ugly aliasing on 64-bit OS X and possibly others
    #draw_quad 0, 0, Gosu::Color::WHITE, 800, 0, Gosu::Color::WHITE,
    #  800, 600, Gosu::Color::WHITE, 0, 600, Gosu::Color::WHITE, 0
    
    @font.draw TEST_STRING, 10, 10, 0
    
    @simple_text.draw 10, 60, 0
    
    @complex_text.draw 10, 160, 0
  end
  
  def button_down(id)
    close if id == Gosu::KbEscape
  end
end

Test.new.show
