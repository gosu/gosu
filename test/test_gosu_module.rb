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
      draw_triangle: [0, 0, 0xff_ffffff, 10, 10, 0xff_ffffff, 0, 10, 0xff_ffffff, 0, :default],
      draw_quad: [0, 0, 0xff_ffffff, 10, 10, 0xff_ffffff, 0, 10, 0xff_ffffff, 0, 10, 0xff_ffffff0, 0, :default],
    }.each do |method, args|
      assert_respond_to Gosu, method

      img = Gosu.record(20, 20) do
        Gosu.send(method, *args)
      end
      assert_equal Gosu::Image, img.class

      # RuntimeError: Gosu::Macro cannot be rendered as Gosu::Bitmap yet
      # that would be awesome. Making "screenshots" in general would greatly
      # extend the possibilities of automated testing.
      # Already requestesd in https://github.com/gosu/gosu/issues/317
      # img.save("/tmp/test.png")
    end
  end


  # TODO: Manipulating the current drawing context
  # clip_to  flush  gl  record  rotate  scale  transform  translate

  def test_misc
    assert_match(/^[a-z]{2}/, Gosu.language)

    assert_equal 0, Gosu.fps
    assert_equal 0, Gosu.milliseconds

    assert_equal 'sans', Gosu.default_font_name

    # TODO: Test if this can be set in the CI, e.g. with xvfb
    # Link: https://docs.travis-ci.com/user/gui-and-headless-browsers/
    # otherwise just check for > 0 as done with the available_* values
    assert_equal 1920, Gosu.screen_height
    assert_equal 1080, Gosu.screen_width

    assert Gosu.available_height > 0
    assert Gosu.available_width > 0

    # Thats just an assumption, not a proven fact
    assert Gosu.screen_height > Gosu.available_height
    assert Gosu.screen_width > Gosu.available_width

    assert_match(/[Aa]/, Gosu.button_id_to_char(Gosu::KB_A))
    assert_equal Gosu::KB_A, Gosu.char_to_button_id("A")

    refute Gosu.button_down?(Gosu::KB_A)
  end

  def test_math
    {
      [ 0, 0] =>   0.0,
      [ 1, 0] =>  90.0,
      [ 1, 1] => 135.0,
      [ 0, 1] => 180.0,
      [-1, 1] => 225.0,
      [-1, 0] => 270.0,
      [-1,-1] => 315.0,
      [ 0,-1] =>   0.0, # 360.0
    }.each do |point, angle|
      assert_equal angle, Gosu.angle(0, 0, *point)
    end

    {
      [90, 95] =>    5.0,
      [90, 85] =>   -5.0,
      [90,269] =>  179.0,
      [90,271] => -179.0,
    }.each do |(angle1, angle2), delta|
      assert_equal delta, Gosu.angle_diff(angle1, angle2)
    end

    # TODO: Check test-vectors
    {
      [ 36.86, 5] => [ 3, -4], # a² + b² = c² | 3² + 4² = 5²
      [  0.0 , 1] => [ 0,  1],
      [ 90.0 , 1] => [ 1,  0],
      [180.0 , 1] => [ 0, -1],
      [-90.0 , 1] => [-1,  0],
    }.each do |(angle,length),(dx, dy)|
      assert_in_delta dx, Gosu.offset_x(angle, length), 0.1
      assert_in_delta dy, Gosu.offset_y(angle, length), 0.1
    end

    # TODO: Add more test-vectors
    {
      [0,0 , 3,4] => 5, # a² + b² = c² | 3² + 4² = 5²
    }.each do |(x1,y1,x2,y2), dist|
      assert_equal dist, Gosu.distance(x1, y1, x2, y2)
    end

    100.times do
      val = Gosu.random(5, 10)
      assert val.is_a?(Float)
      assert val >= 5
      assert val < 10
    end
  end
end
