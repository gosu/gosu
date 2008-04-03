require '../gosu'

class TextInputWindow < Gosu::Window
  def initialize
    super(300, 200, false)
    self.caption = "Text Input Example"
    @font = Gosu::Font.new(self, Gosu::default_font_name, 20)
    self.text_input = Gosu::TextInput.new
  end
  
  def update
    GC.start
  end
  
  def draw
    sel_x = 10 + @font.text_width(text_input.text[0...text_input.selection_start])
    pos_x = 10 + @font.text_width(text_input.text[0...text_input.caret_pos])
    
    draw_quad(sel_x, 100, 0xcc0000ff, pos_x, 100, 0xcc0000ff,
              sel_x, 120, 0xcc0000ff, pos_x, 120, 0xcc0000ff, 0)
              
    draw_line(pos_x, 100, 0xffffffff, pos_x, 120, 0xffffffff, 0)
    
    @font.draw(text_input.text, 10, 100, 0)
  end
end

TextInputWindow.new.show
