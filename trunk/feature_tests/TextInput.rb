require '../gosu'

class Test < Gosu::Window
  def initialize
    super(300, 200, false, 20)
    @font = Gosu::Font.new(self, "Helvetica", 20)
    self.text_input = Gosu::TextInput.new
  end
  
  def draw
    text = self.text_input.text
    
    @sel = @pos = 0
    
    sel_x = 10 + @font.text_width(text[0...@sel])
    pos_x = 10 + @font.text_width(text[0...@pos])
    
    draw_quad(sel_x, 100, 0x990000ff, pos_x, 100, 0x990000ff,
              sel_x, 120, 0x990000ff, pos_x, 120, 0x990000ff)

    @font.draw(text, 10, 100, 0)

    draw_line(pos_x, 100, 0xffffffff, pos_x, 120, 0xffffffff, 0)
  end
end

Test.new.show
