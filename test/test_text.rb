# Encoding: UTF-8
require_relative "test_helper"

class TestText < Minitest::Test
  include TestHelper
  
  STRINGS = {
    "unicode"    => "Stößken! 「己所不欲，勿施於人」 กรุงเทพมหานคร ≠ 부산 ✓",
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
      # Gosu.register_entity "entity", Gosu::Image.new("alpha-bmp24.bmp")

      Dir.mktmpdir do |tmpdir|
        STRINGS.each do |key, string|
          OPTION_SETS.each_with_index do |options, i|
            # Prepend <c=000000> to each string because white-on-translucent images are hard
            # to view (on macOS at least).
            image = Gosu::Image.from_text("<c=000000>#{string}", 41, options)
            
            expected = "text-#{key}-#{i}.png" 
            output_filename = File.join(tmpdir, expected)
            
            image.save output_filename
            assert_equal Gosu::Image.new(expected), image
          end
        end
      end
    end
  end
end
