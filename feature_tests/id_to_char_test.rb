$LOAD_PATH << '../lib'
require 'gosu'

class Test < Gosu::Window
  def button_down id
    char = self.button_id_to_char(id)
    self.caption += char if char and not ["\n", "\r"].include? char
  end
  
  def draw
    return if self.caption.empty?
    
    last_char = self.caption[-1, 1]
    id_for_last_char = self.char_to_button_id(last_char)
    constant_for_last_char = Gosu.constants.find { |c| Gosu.const_get(c) == id_for_last_char }
    
    @font ||= Gosu::Font.new(self, "Comic Sans MS", 40)
    @font.draw "Constant for last character is #{constant_for_last_char}", 0, 0, 0
  end
end

Test.new(640, 200, false).show
