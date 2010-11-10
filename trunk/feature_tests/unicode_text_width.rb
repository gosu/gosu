require '../lib/gosu'

class Test < Gosu::Window
  def initialize
    super(800, 600, false)
	font = Gosu::Font.new(self, "Arial", 20)
	self.caption = "U=#{font.text_width 'U'} Ü=#{font.text_width 'Ü'}"
  end
end

Test.new.show
