require '../gosu'

class Test < Gosu::Window
  def initialize
    super(300, 200, false, 20)
    @font = Gosu::Font.new(self, "Arial", 20)
    self.text_input = Gosu::TextInput.new
  end
  
  def draw
    sel_x = 10 + @font.text_width(text_input.text[0...text_input.selection_start])
    pos_x = 10 + @font.text_width(text_input.text[0...text_input.caret_pos])
    
    draw_quad(sel_x, 100, 0xcc0000ff, pos_x, 100, 0xcc0000ff,
              sel_x, 120, 0xcc0000ff, pos_x, 120, 0xcc0000ff)
              
    draw_line(pos_x, 100, 0xffffffff, pos_x, 120, 0xffffffff, 0)    
    
    @font.draw(text_input.text, 10, 100, 0)
  end
end

Test.new.show
