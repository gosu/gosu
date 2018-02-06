# Encoding: UTF-8
require_relative "test_helper"

class TestText < Minitest::Test
  include TestHelper
  
  STRINGS = {
    # All of these strings are still horribly broken in Gosu.
    # For now, the value of these tests is that all text is guaranteed to render the same across
    # operating systems.
    "unicode"    => "Grüße vom Test! 「己所不欲，勿施於人」 กรุงเทพมหานคร ≠ 부산 ✓",
    "whitespace" => "$ ls\n  .\t..\tfoo\r\n  bar\tqux        ",
    "markup"     => "<b>Bold, <u>underlined &amp; <i>italic. <c=4400ff>How <c=0f3>about</c> colors?</c></i></u>&lt;&entity;&gt;</b>",
    # This string will remain broken in the foreseeable future, but at least Gosu should be able
    # to correctly split it into grapheme clusters.
    # Maybe we could use this for testing later: https://github.com/emojione/emojione/
    "emoji"      => "Chinese Zodiac: '🐒🐓🐕🐖🐀🐂🐆🐇🐉🐍🐎🐑'[y%12]. ZWJ sequences: 👨🏿‍⚕️ 👨‍👨‍👦",
  }
  
  OPTION_SETS = [
    { font: "./daniel.ttf", align: :right, width: 139 },
    { font: "./daniel.otf", align: :center, spacing: 10 },
  ]
  
  def test_text_rendering
    Dir.chdir media_path do
      # TODO: This API exists in C++, but is missing from Ruby/Gosu.
      # Gosu.register_entity "entity", Gosu::Image.new("alpha-bmp24.bmp")

      STRINGS.each do |key, string|
        OPTION_SETS.each_with_index do |options, i|
          expected_filename = "text-#{key}-#{i}.png"

          # Prepend <c=r00> to each string because white-on-translucent images are hard
          # to view (on macOS at least).
          # TODO: Color markup seems to be completely broken for Image.from_text?
          image = Gosu::Image.from_text("<c=ff0000>#{string}", 41, options)
          assert_equal Gosu::Image.new(expected_filename), image
        end
      end
    end
  end
end
