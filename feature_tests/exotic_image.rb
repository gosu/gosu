$LOAD_PATH << '../lib'
require 'gosu'

class Test < Gosu::Window
  def initialize 
    super(640, 480, false)
    @image = Gosu::Image.new(self, "media/Test.psd", false)
  end
  
  def draw
    @image.draw 0, 0, 0
  end
end

Test.new.show
