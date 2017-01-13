# Encoding: UTF-8

require "minitest/autorun"
require "gosu" unless defined? Gosu

class TestGosuModule < Minitest::Test
  def test_drawing_primitives
    {
      # args: [x, y, width, height, c, z, mode]
      draw_rect: [0, 0, 10, 10, 0xff_ffffff, 0, :default],

      # args: [x, y, c]{2,4}, z, mode
      draw_line: [0, 0, 0xff_ffffff, 10, 10, 0xff_ffffff, 0, :default],
      draw_quad: [0, 0, 0xff_ffffff, 10, 10, 0xff_ffffff, 0, 10, 0xff_ffffff, :default],
      draw_quad: [0, 0, 0xff_ffffff, 10, 10, 0xff_ffffff, 0, 10, 0xff_ffffff, 0, 10, 0xff_ffffff0, 0, :default],
    }.each do |method, args|
      assert_respond_to Gosu, method
      error = assert_raises ::RuntimeError do
        Gosu.send(method, *args)
      end
      assert_match /There is no rendering queue for this operation/, error.message
    end
  end
end
