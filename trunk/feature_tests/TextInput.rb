require '../gosu'

class Test < Gosu::Window
  def initialize
    super(300, 200, false, 20)
    @font = Gosu::Font.new(self, "Helvetica", 20)
    @text = ""
    @sel = 0
    @pos = 0
  end
    
  def button_down(id)
    return close if id == Gosu::KbEscape
    
    case id
    when Gosu::KbUp, Gosu::KbHome:
      @pos = 0
      @sel = @pos unless button_down? Gosu::KbLeftShift or button_down? Gosu::KbRightShift
      
    when Gosu::KbDown, Gosu::KbEnd
      @pos = @text.length
      @sel = @pos unless button_down? Gosu::KbLeftShift or button_down? Gosu::KbRightShift
    
    when Gosu::KbLeft:
      @pos -= 1 unless @pos == 0
      @sel = @pos unless button_down? Gosu::KbLeftShift or button_down? Gosu::KbRightShift
      
    when Gosu::KbRight:
      @pos += 1 unless @pos == @text.length
      @sel = @pos unless button_down? Gosu::KbLeftShift or button_down? Gosu::KbRightShift
      
    when Gosu::KbDelete:
      if @sel != @pos then
        @text[[@sel, @pos].min ... [@sel, @pos].max] = ""
        @sel = @pos = [@sel, @pos].min
      elsif @pos < @text.length then
        @text[@pos] = ""
      end
      
    when Gosu::KbBackspace:
      if @sel != @pos then
        @text[[@sel, @pos].min ... [@sel, @pos].max] = ""
        @sel = @pos = [@sel, @pos].min
      elsif @pos > 0 then
        @pos -= 1
        @sel -= 1
        @text[@pos] = ""
      end
      
    else
      ch = button_id_to_char(id)
      return if ch.empty?
      ch.upcase! if button_down? Gosu::KbLeftShift or button_down? Gosu::KbRightShift
      @text.insert @pos, ch
      @pos += 1
      @sel += 1
    end
  end
  
  def draw
    sel_x = 10 + @font.text_width(@text[0...@sel])
    pos_x = 10 + @font.text_width(@text[0...@pos])
    
    draw_quad(sel_x, 100, 0x990000ff, pos_x, 100, 0x990000ff,
              sel_x, 120, 0x990000ff, pos_x, 120, 0x990000ff)

    @font.draw(@text, 10, 100, 0)

    draw_line(pos_x, 100, 0xffffffff, pos_x, 120, 0xffffffff, 0)
  end
end

Test.new.show
