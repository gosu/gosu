# Encoding: UTF-8
require_relative "test_helper"

class TestImage < Minitest::Test
  include TestHelper
  
  def test_image_from_blob
    width = height = 141
    # This makes the loop below a bit prettier (-radius..+radius).
    radius = width / 2

    # Opaque blue as an RGBA string (0x0000ffff).
    inside = 0x00.chr + 0x00.chr + 0xff.chr + 0xff.chr
    # Transparent blue as an RGBA string (0x0000ff00).
    outside = 0x00.chr + 0xff.chr + 0xff.chr + 0x00.chr

    # This creates a binary string that contains a circle, thanks to the
    # Pythagorean theorem.
    rgba = (-radius..+radius).map do |y|
      # 'x' is the distance from the center to the edges edge of the circle.
      x = Math.sqrt(radius ** 2 - y ** 2).round
      outside_circle = outside * (radius - x)
      inside_circle = inside * (x * 2 + 1)
      outside_circle + inside_circle + outside_circle
    end.join
    
    image = Gosu::Image.from_blob(width, height, rgba)
    
    # Font#draw_rot has been deprecated a long time ago.
    assert_image_matches "test_image/from_blob", image, 1.00
  end
end
