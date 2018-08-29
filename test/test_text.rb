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
    "markup"     => "<b>Bold, <u>underlined &amp; <i>italic. <c=4400ff>How <c=0f3>about</c> colors?</c></i></u>&lt;&gt;</b>",
    # This string will remain broken in the foreseeable future, but at least Gosu should be able
    # to correctly split it into grapheme clusters.
    # Maybe we could use this for testing later: https://github.com/emojione/emojione/
    "emoji"      => "Chinese Zodiac: '🐒🐓🐕🐖🐀🐂🐆🐇🐉🐍🐎🐑'[y%12]. ZWJ sequences: 👨🏿‍⚕️ 👨‍👨‍👦",
  }
  
  OPTION_SETS = [
    { font: "./daniel.ttf", align: :right, width: 139 },
    { font: "./daniel.otf", align: :center, spacing: 10 },
  ]
  
  STRINGS.each do |key, string|
    OPTION_SETS.each_with_index do |options, i|
      define_method("test_text_#{key}_#{i}") do
        Dir.chdir media_path do
          expected_filename = "text-#{key}-#{i}.png"

          # Prepend <c=f00> to each string because white-on-translucent images are hard
          # to view (at least on macOS).
          image = Gosu::Image.from_text("<c=ff0000>#{string}", 41, options)
          image.save "debug-#{expected_filename}" if ENV["DEBUG"]
          expected = Gosu::Image.new(expected_filename)
          assert_equal expected, image
        end
      end
    end
  end
end
