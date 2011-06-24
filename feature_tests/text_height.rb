$LOAD_PATH << '../lib'
require 'gosu'
include Gosu

class TextHeightTest < Window
  def initialize
    super 400, 100, false
    @system_text = Image.from_text(self, "sŸstem_", "Arial", 80)
    @ttf_text    = Image.from_text(self, "TTF Ïmporty", "media/Vera.ttf", 80)
  end

  def draw
    draw_quad  10, 10, Color::RED,
              390, 10, Color::RED,
               10, 90, Color::RED,
              390, 90, Color::RED, 0
    @system_text.draw 10, 10, 0
    @ttf_text.draw    10, 10, 0
  end
end

TextHeightTest.new.show

