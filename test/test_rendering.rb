# Encoding: UTF-8
require_relative "test_helper"

class TestMacros < Minitest::Test
  include TestHelper

  class RenderWindow < Gosu::Window
    def self.draw(w,h,&block)
      win = new(w,h)
      win.define_singleton_method :draw, &block
      win
    end
  end

  def test_lines_with_thickness
    line_length = 250
    window = RenderWindow.draw(line_length*2, line_length*2) do
      (0..359).step(10).each do |angle|
        c = 0xFF_a0a000 + (angle*2)
        dx = Gosu.offset_x(angle, line_length)
        dy = Gosu.offset_y(angle, line_length)
        Gosu.draw_line(line_length, line_length, c, line_length+dx, line_length+dy, c, 0, :default, angle/10.0+1)
      end
    end
    assert_screenshot_matches window, 'circled-lines.png'
  end
end
