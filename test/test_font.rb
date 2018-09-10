# Encoding: UTF-8
require_relative "test_helper"

class TestFont < Minitest::Test
  include TestHelper

  SINGLE_PIXEL = begin
    image_data = Struct.new(:columns, :rows, :to_blob)
    single_pixel_data = image_data.new(1, 1, "\xff\xff\xff\xff")
    Gosu::Image.new(single_pixel_data)
  end
  
  def test_markup_parsing
    font = Gosu::Font.new(20)
    # markup_width and text_width will return the same thing, for compatibility, until Gosu 1.0.
    assert_equal 0, font.markup_width("<b>")
    assert_equal 0, font.text_width("<b>")
  end
  
  def test_custom_characters
    font = Gosu::Font.new(20)
    font["a"] = SINGLE_PIXEL
    font["b"] = SINGLE_PIXEL
    font["c"] = SINGLE_PIXEL
    # Five square letters will be scaled up to 20px.
    assert_equal 100, font.text_width("abcba")
  end
  
  def test_constructor_args
    bold_font = Gosu::Font.new(7, bold: true)
    regular_font = Gosu::Font.new(7, bold: false)
    refute_equal bold_font.text_width("IJK"), regular_font.text_width("IJK")
    assert_equal bold_font.text_width("IJK"), regular_font.text_width("<b>IJK</b>")
    assert_equal bold_font.text_width("</b>IJK"), regular_font.text_width("IJK")
  end
  
  def test_draw_and_draw_rel
    font = Gosu::Font.new(10, name: media_path("daniel.otf"))
    
    assert_output_matches "test_font/draw_markup", 0.99, [200, 50] do
      font.draw_markup "Hi! <c=f00>Red.\r\nNew   line! Äöß\n", 5, 5, -6, 1, 2.0, 0xff_ff00ff, :add
    end
    
    assert_output_matches "test_font/draw_markup_rel", 0.99, [100, 100] do
      font.draw_markup_rel "<c=000>I &lt;3 Ruby/Gosu!\n", 50, 50, 5, 0.4, -2
    end
  end

  def test_deprecated_features
    font = Gosu::Font.new(20, name: media_path("daniel.ttf"))
    
    # The scale_x parameter to Font#text_width has been deprecated in Gosu 0.14.0.
    assert_equal 5 * font.text_width("Hello"), font.text_width("Hello", 5)
    
    # Font#draw and Font#draw_rel have been deprecated in Gosu 0.14.0.
    assert_respond_to font, :draw
    assert_respond_to font, :draw_rel
    
    # Font#draw_rot has been deprecated a long time ago.
    assert_output_matches "test_font/draw_rot", 0.99, [200, 200] do
      font.draw_rot "<b>Hello</b> <c=888>World!</c>", 10, 0, 0, 45, 0.7, 3, 0xff_ff00ff, :add
    end
  end

end
