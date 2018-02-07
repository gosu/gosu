# Encoding: UTF-8
require_relative "test_helper"

class TestFont < Minitest::Test
  include TestHelper

  SINGLE_PIXEL = begin
    image_data = Struct.new(:columns, :rows, :to_blob)
    single_pixel_data = image_data.new(1, 1, "\xff\xff\xff\xff")
    Gosu::Image.new(single_pixel_data)
  end
  
  def test_font_constructor
    # Font.text_width and Font.draw should request to render each the letter only once when it is
    # first needed.
    requested_strings = []
    font = Gosu::Font.new(20) { |str| requested_strings << str; SINGLE_PIXEL }
    font.text_width("Hallo Welt!") # implicitly renders each letter
    assert_equal %w(H a l o W e t !), requested_strings
  end
  
  
  STRING_LENGTHS = {
    "" => 0,
    " " => 1,
    "Hi" => 2,
    "Öäü" => 3,
    
    "🐒🐓🐕🐖🐀🐂🐆🐇🐉🐍🐎🐑" => 12, # 12 "normal" emoji
    "👩🏻‍⚕️" => 1, # one emoji - https://emojipedia.org/female-health-worker-type-1-2/
  }
  
  def test_font_grapheme_clusters
    requested_strings = []
    assert_equal 1, SINGLE_PIXEL.width
    font = Gosu::Font.new(20) { |str| requested_strings << str; SINGLE_PIXEL }

    # Ensure that Font considers the right number of UTF-8 bytes a single character to render.
    STRING_LENGTHS.each do |str, length|
      assert_equal length, font.text_width(str)
    end
    
    assert_equal %w( H i Ö ä ü 🐒 🐓 🐕 🐖 🐀 🐂 🐆 🐇 🐉 🐍 🐎 🐑 👩🏻‍⚕️), requested_strings
  end
end
