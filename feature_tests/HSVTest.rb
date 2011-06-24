$LOAD_PATH << '../lib'
require 'gosu'

class Test < Gosu::Window
  def initialize 
    super(640, 480, false, 100)
    @c1 = Gosu::Color.new(0xffff0000)
    @c2 = Gosu::Color.new(0xff00ff00)
    @c3 = Gosu::Color.new(0xff0000ff)
    
    # should work
    @c1 == @c1 or raise "false negative"
    # crashes when doing it naively with SWIG, but should work now
    @c1 == 'not a color' and raise "false positive"
    # should also work, MAGIC
    @c1 == 0xffff0000 or raise "false indirect negative"
  end

  def update
    @c1.hue = ((@c1.hue + 3) % 360)
    @c2.saturation = ((@c2.saturation + 0.04) % 1.0)
    @c3.value = ((@c3.value + 0.05) % 1.0)
  end

  def draw
    draw_triangle(320, 0, @c1,
                  0, 480, @c2,
                  640, 480, @c3,
                  0)
  end
end

Test.new.show
