# Encoding: UTF-8
require_relative "test_helper"

class TestColor < Minitest::Test
  def test_predefined_colors
    [:NONE, :BLACK, :GRAY, :WHITE, :AQUA, :RED, :GREEN, :BLUE, :YELLOW, :FUCHSIA, :CYAN].each do |base_color|
      assert Gosu::Color.const_get(base_color).is_a? Gosu::Color
    end
  end

  def test_color_creation
    # ARGB
    assert_equal Gosu::Color::CYAN,    Gosu::Color.new(0xff_00ffff)
    assert_equal Gosu::Color::RED,     Gosu::Color.new( 0xff, 0xff, 0x00, 0x00)
    assert_equal Gosu::Color::GREEN,   Gosu::Color.argb(0xff, 0x00, 0xff, 0x00)
    assert_equal Gosu::Color::BLUE,    Gosu::Color.rgba(0x00, 0x00, 0xff, 0xff)

    # (A)HSV
    assert_equal Gosu::Color::FUCHSIA, Gosu::Color.from_hsv(     300, 1.0, 1.0)
    assert_equal Gosu::Color::YELLOW,  Gosu::Color.from_ahsv(255, 60, 1.0, 1.0)
  end

  def test_color_atttributes_and_types
    color = Gosu::Color.new(255, 255, 255, 255)
    [:alpha, :red, :green, :blue].each do |attr|
      assert_respond_to color, attr
      assert_kind_of Integer, color.send(attr), "Color##{attr} should be kind of Integer"
    end

    [:hue, :saturation, :value].each do |attr|
      assert_respond_to color, attr
      assert_kind_of Float, color.send(attr), "Color##{attr} should be kind of Float"
    end
  end

  def test_color_attribute_ranges
    # alpha, red, green, blue are clamped to 0..255.
    assert_equal Gosu::Color::WHITE, Gosu::Color.new(300, 300, 300, 300)
    assert_equal Gosu::Color::NONE,  Gosu::Color.new(-50, -50, -50, -50)

    # hue wraps(!) at 360, so 361 is the same as 1.
    assert_equal Gosu::Color.from_ahsv(100, 1, 1.0, 1.0), Gosu::Color.from_ahsv(100, 361, 1.0, 1.0)

    # saturation and value are clamped to 0.0..1.0 (since commit 01120e92f7a3).
    assert_equal Gosu::Color.from_ahsv(100, 1, 1.0, 1.0), Gosu::Color.from_ahsv(100, 361,  2.0,  2.0)
    assert_equal Gosu::Color.from_ahsv(100, 1, 0.0, 0.0), Gosu::Color.from_ahsv(100, 361, -1.0, -1.0)
  end

  def test_dup_and_gl
    refute_equal Gosu::Color::BLACK.object_id, Gosu::Color::BLACK.dup.object_id
    assert_equal 0xff00ff00, Gosu::Color::GREEN.gl
  end

  # introduced by #316, requested by #333 (@shawn42)
  # this is not documented in rdoc/gosu.rb
  def test_alias
    assert Gosu::Color::AQUA.eql?(Gosu::Color::CYAN)
    assert_equal Gosu::Color::AQUA.hash, Gosu::Color::AQUA.gl
  end
end
