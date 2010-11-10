require '../lib/gosu'

class Test < Gosu::Window
  def initialize
    super(800, 600, false)
	@font = Gosu::Font.new(self, "Arial", 20)
	self.text_input = Gosu::TextInput.new
  end
  
  def update
	self.caption = "#{text_input.text}=#{@font.text_width text_input.text}"
  end
end

Test.new.show
