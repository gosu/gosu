# Encoding: UTF-8
require_relative "test_helper"

class TestGosuModule < Minitest::Test
  include TestHelper

  def test_drawing_primitives
    {
      # args: [x, y, width, height, c, z, mode]
      draw_rect: [0, 0, 10, 10, 0xff_ffffff, 0, :default],

      # args: [x, y, c]{2,4}, z, mode
      draw_line: [0, 0, 0xff_ffffff, 10, 10, 0xff_ffffff, 0, :default],
      draw_triangle: [0, 0, 0xff_ffffff, 10, 10, 0xff_ffffff, 0, 10, 0xff_ffffff, 0, :default],
      draw_quad: [0, 0, 0xff_ffffff, 10, 10, 0xff_ffffff, 0, 10, 0xff_ffffff, 0, 10, 0xff_ffffff, 0, :default],
    }.each do |method, args|
      assert_respond_to Gosu, method

      img = Gosu.record(20, 20) do
        Gosu.send(method, *args)
      end
      assert_equal Gosu::Image, img.class
    end
  end


  # TODO: Manipulating the current drawing context
  # clip_to  flush  gl  record  rotate  scale  transform  translate

  def test_misc
    assert_match(/^[a-z]{2}/, Gosu.language)

    # TODO: This test can cause trouble if run after other tests, which might have updated Gosu.fps.
    # assert_equal 0, Gosu.fps, "Gosu.fps should be 0 as there is nothing drawn"

    first_call = Gosu.milliseconds
    assert_kind_of Integer, Gosu.milliseconds, "Gosu.milliseconds should return an integer"
    sleep 0.2
    assert first_call < Gosu.milliseconds, "Gosu.milliseconds should increase over time"

    assert Gosu.default_font_name.is_a? String

    assert Gosu.available_height > 0
    assert Gosu.available_width > 0

    assert Gosu.screen_height >= Gosu.available_height
    assert Gosu.screen_width >= Gosu.available_width

    assert_match("g", Gosu.button_id_to_char(Gosu::KB_G))
    assert_equal Gosu::KB_G, Gosu.char_to_button_id("g")
    assert_equal Gosu::KB_G, Gosu.char_to_button_id("G")

    refute Gosu.button_down?(Gosu::KB_A)
  end

  def test_angle
    {
      [ 0,  0] =>   0.0,
      [ 1,  0] =>  90.0,
      [ 1,  1] => 135.0,
      [ 0,  1] => 180.0,
      [-1,  1] => 225.0,
      [-1,  0] => 270.0,
      [-1, -1] => 315.0,
      [ 0, -1] =>   0.0, # 360.0
    }.each do |point, angle|
      assert_equal angle, Gosu.angle(0, 0, *point)
    end
  end

  def test_angle_diff
    {
      [90,  95] =>    5.0,
      [90,  85] =>   -5.0,
      [90, 269] =>  179.0,
      [90, 271] => -179.0,
    }.each do |(angle1, angle2), delta|
      assert_equal delta, Gosu.angle_diff(angle1, angle2)
    end
  end

  def test_offset
    {
      [ 36.86, 5] => [ 3, -4], # a² + b² = c² | 3² + 4² = 5²
      [  0.0,  1] => [ 0, -1],
      [ 90.0,  1] => [ 1,  0],
      [180.0,  1] => [ 0,  1],
      [-90.0,  1] => [-1,  0],
    }.each do |(angle,length),(dx, dy)|
      assert_in_delta dx, Gosu.offset_x(angle, length), 0.1,
        "Angle: #{angle}, Length: #{length} | offset_x should be #{dx} but is #{Gosu.offset_x(angle, length)}"
      assert_in_delta dy, Gosu.offset_y(angle, length), 0.1,
        "Angle: #{angle}, Length: #{length} | offset_y should be #{dy} but is #{Gosu.offset_x(angle, length)}"
    end
  end

  # TODO: Add more test vectors
  def test_distance
    {
      [0,0 , 3,4] => 5, # a² + b² = c² | 3² + 4² = 5²
      [-2,-3 , -4, 4] => 7.28,
    }.each do |(x1,y1,x2,y2), dist|
      assert_in_delta dist, Gosu.distance(x1, y1, x2, y2), 0.1
    end
  end

  def test_random
    100.times do
      val = Gosu.random(5, 10)
      assert val.is_a?(Float)
      assert val >= 5
      assert val < 10
    end
  end
  
  SIZES = [25, 50, 500]

  def test_render
    # Gosu.render does not work on Appveyor.
    skip_on_appveyor
    
    SIZES.each do |size|
      assert_output_matches("test_gosu_module/triangle-#{size}", 0.9, [size, size]) do
        Gosu.draw_triangle(0, 0, 0xff_ff0000, size, 0, 0xff_00ff00, size, size, 0xff_0000ff, 0)
      end
    end
  end
end
