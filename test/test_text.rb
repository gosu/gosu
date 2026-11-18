require_relative "test_helper"

class TestText < Minitest::Test
  include TestHelper
  
  STRINGS = {
    # All of these strings are still horribly broken in Gosu.
    # The only thing that these tests verify is that they're equally broken on
    # all operating systems.
    "unicode"    => "Grüße vom Test!",
    "whitespace" => "$ ls\n  .\t..\tfoo\r\n  bar\tqux        ",
    "markup"     => "<b>Bold, <u>underlined &amp; <i>italic. <c=4400ff>How <c=0f3>about</c> colors?</c></i></u>&lt;&gt;</b>",
    # All Emoji should be invisible for now.
    # "emoji"      => "Chinese Zodiac: '🐒🐓🐕🐖🐀🐂🐆🐇🐉🐍🐎🐑'[y%12]. ZWJ sequences: 👨🏿‍⚕️ 👨‍👨‍👦",
  }
  
  OPTION_SETS = [
    { font: TestHelper.media_path("daniel.ttf"), align: :right, width: 139 },
    { font: TestHelper.media_path("daniel.otf"), align: :center, spacing: 10 },
  ]
  
  STRINGS.each do |key, string|
    OPTION_SETS.each_with_index do |options, i|
      define_method("test_text_#{key}_#{i}") do
        Dir.chdir File.join(File.dirname(__FILE__), "test_text") do
          # Prepend <c=f00> to each string because white-on-translucent images are hard
          # to view (at least on macOS).
          image = Gosu::Image.from_markup("<c=ff0000>#{string}", 41, **options)
          
          assert_image_matches "test_text/text-#{key}-#{i}", image, 1.00
        end
      end
    end
  end

  def test_non_string_text
    assert_equal Gosu::Image.from_text("123", 20), Gosu::Image.from_text(123, 20)
    assert_equal Gosu::Image.from_markup("123", 20), Gosu::Image.from_markup(123, 20)
    assert_equal Gosu::Image.from_text("", 20), Gosu::Image.from_text(nil, 20)
  end
end
