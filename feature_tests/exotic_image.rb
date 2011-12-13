$LOAD_PATH << '../lib'
require 'gosu'

class Test < Gosu::Window
  def initialize 
    super(640, 480, false)
    @images = %w(psd jpg).map { |ext| Gosu::Image.new(self, "media/Test.#{ext}", false) }
  end
  
  def draw
    y = 0
    @images.each do |image|
      image.draw 0, y, 0
      y += image.height
    end
  end
end

Test.new.show
