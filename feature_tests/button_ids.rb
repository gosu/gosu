$LOAD_PATH << '../lib'
require 'gosu'

class ButtonIDs < Gosu::Window
  def initialize
    super 400, 600, false
    
    @constants = []
  end
  
  def button_down(id)
    @constants << "0x#{id.to_s(16)} = #{Gosu.constants.find { |c| c.upcase != c and Gosu.const_get(c) == id }}"
  end
  
  def draw
    @font ||= Gosu::Font.new(self, Gosu::default_font_name, 20)

    @constants.reverse.each_with_index do |constant, idx|
      @font.draw constant, 10, 10 + idx * 20, 0
    end
    
    y = -10
    Gosu.constants.each do |c| 
      if id = Gosu.const_get(c) and id.is_a? Fixnum and button_down? id then
        @font.draw c.to_s, 200, y += 20, 0
      end
    end
  end
end

ButtonIDs.new.show
