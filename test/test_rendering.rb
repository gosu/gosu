# Encoding: UTF-8
require_relative "test_helper"

class TestMacros < Minitest::Test
  include TestHelper

  class RenderWindow < Gosu::Window
    def self.draw(w, h, &block)
      win = new(w, h)
      win.define_singleton_method :draw, &block
      win
    end
  end

  def test_lines
    line_length = 250
    window = RenderWindow.draw(line_length * 2, line_length * 2) do
      (0..359).step(10).each do |angle|
        c = 0xff_a0a000 + (angle * 2)
        dx = Gosu.offset_x(angle, line_length)
        dy = Gosu.offset_y(angle, line_length)
        Gosu.draw_line(line_length, line_length, c, line_length + dx, line_length + dy, c, 0, :default)
      end
    end
    assert_screenshot_matches window, "circled-lines.png"
  end

  def test_rect_from_lines
    window = RenderWindow.draw(200, 100) do
      c = 0x80_ff0000
      Gosu.draw_line(10, 10, c, 90, 10, c)
      Gosu.draw_line(90, 10, c, 90, 90, c)
      Gosu.draw_line(90, 90, c, 10, 90, c)
      Gosu.draw_line(10, 90, c, 10, 10, c)

      c = 0x80_00ff00
      Gosu.draw_line(110, 10, c, 190, 10, c, 0, :default, 10)
      Gosu.draw_line(190, 10, c, 190, 90, c, 0, :default, 10)
      Gosu.draw_line(190, 90, c, 110, 90, c, 0, :default, 10)
      Gosu.draw_line(110, 90, c, 110, 10, c, 0, :default, 10)
    end
    assert_screenshot_matches window, "rectangle-lines.png"
  end
end
