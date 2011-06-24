# Encoding: UTF-8

$LOAD_PATH << '../lib'
require 'gosu'

class Test < Gosu::Window
  def initialize
    super(800, 600, false)
	@font = Gosu::Font.new(self, "Arial", 20)
	self.text_input = Gosu::TextInput.new
	#self.caption = "Ü=#{@font.text_width 'Ü'}"
  end
  
  def update
	self.caption = "#{text_input.text}=#{@font.text_width text_input.text}"
  end
end

Test.new.show
