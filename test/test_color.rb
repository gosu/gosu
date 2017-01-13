require "minitest/autorun"
require "gosu" unless defined? Gosu

class TestConstants < Minitest::Test
  def test_predefined_colors
    [:NONE, :BLACK, :GRAY, :WHITE, :AQUA, :RED, :GREEN, :BLUE, :YELLOW, :FUCHSIA, :CYAN].each do |base_color|
      assert Gosu::Color.const_defined?(base_color)
    end
  end

  def test_color_creation
    # ARGB
    assert_equal Gosu::Color::RED,   Gosu::Color.new( 0xff, 0xff, 0x00, 0x00)
    assert_equal Gosu::Color::GREEN, Gosu::Color.argb(0xff, 0x00, 0xff, 0x00)
    assert_equal Gosu::Color::BLUE,  Gosu::Color.rgba(0x00, 0x00, 0xff, 0xff)

    # (A)HSV
    assert_equal Gosu::Color::FUCHSIA, Gosu::Color.from_hsv(     300, 1.0, 1.0)
    assert_equal Gosu::Color::YELLOW,  Gosu::Color.from_ahsv(255, 60, 1.0, 1.0)
  end

  def test_color_atttributes_and_types
    color = Gosu::Color.new(255,255,255,255)
    [:alpha, :red, :green, :blue, :hue].each do |attr|
      assert_respond_to color, attr
      assert_kind_of Integer, color.send(attr), "Color##{attr} should be kind of Integer"
    end

    [:saturation, :value].each do |attr|
      assert_respond_to color, attr
      assert_kind_of Float, color.send(attr), "Color##{attr} should be kind of Float"
    end
  end

  def test_color_attribute_ranges
    # atm this automatically sets the values to max, min if the exceed the range ...
    assert_equal Gosu::Color::WHITE, Gosu::Color.new(300, 300, 300, 300)
    assert_equal Gosu::Color::NONE,  Gosu::Color.new(-50, -50, -50, -50)

    # ... but this does not. I'm actually in favor of raising and exception if either
    # of the creation methods get invalid values or at least let them behave them# same.
    assert_equal Gosu::Color::WHITE, Gosu::Color.from_ahsv(500,  2,  2,  2)
    assert_equal Gosu::Color::NONE,  Gosu::Color.from_ahsv(-50, -5, -5, -5)

    # Not sure about this one, but in some programs (including gimp) saturation and value
    # accept integers in range 0..100 so we might support this?
    assert_equal Gosu::Color.from_ahsv(255, 300, 100, 100), Gosu::Color.from_ahsv(255, 300, 1.0, 1.0)
  end

  def test_dup_and_gl
    refute_equal Gosu::Color::BLACK.object_id, Gosu::Color::BLACK.dup.object_id
    assert_equal 0xff00ff00, Gosu::Color::GREEN.gl
  end

  # introduced by #316, requested by #316 (@shawn42)
  # this is not documented in rdoc/gosu.rb
  def test_alias
    assert Gosu::Color::AQUA.eql?(Gosu::Color::CYAN)
    assert_equal Gosu::Color::AQUA.hash, Gosu::Color::AQUA.gl
  end
end
