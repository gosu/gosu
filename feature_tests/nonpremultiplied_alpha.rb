# Encoding: UTF-8

# Require cutting-edge development Gosu for testing.
$LOAD_PATH << '../lib'
require 'gosu'

class Gosu::Image
  def get_pixel x, y
    x, y = x.to_i, y.to_i
    if (0...width).include? x and (0...height).include? y then
      to_blob[((y * width) + x) * 4, 4].unpack('C4').map { |ch| ch.to_s(16) }.join('|')
    else
      "(not in image)"
    end
  end
end

# If Gosu accidentally premultiplies alpha (happened on OS X) the white sphere
# will fade to dark gray (have a black halo) except in the center.
# If Gosu works correctly, the color should be __ffffff everywhere.

class NonpremultipliedAlphaTest < Gosu::Window
  def initialize
    super 400, 400, false
    @image = Gosu::Image.new(self, "media/WhiteAlpha.png", false)
  end
  
  def draw
    draw_quad 0, 0, Gosu::Color::BLUE, 800, 0, Gosu::Color::BLUE,
      0, 600, Gosu::Color::BLUE, 800, 600, Gosu::Color::BLUE, 0
    @image.draw 0, 0, 0, 8, 8
  end
  
  def update
    self.caption = "Under cursor: #{@image.get_pixel mouse_x / 8, mouse_y / 8}"
  end
  
  def needs_cursor?
    true
  end
end

NonpremultipliedAlphaTest.new.show
