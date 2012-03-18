$LOAD_PATH << '../lib'
require 'gosu/preview'

DEST = [[12, 295], [300, 265], [23, 42], [101, 11]]

class FindMatrix < Gosu::Window
  def initialize
    super 500, 300
    
    self.caption = "Macro draw_as_quad - hold <tab> to to see Image draw_as_quad"
    
    @image = Gosu::Image.new('/Users/jlnr/Pictures/Internetz/sadsadrobot.jpg')
    @macro = record(@image.width, @image.height) { @image.draw 0, 0, 0 }
  end
  
  def draw
    Gosu::draw_quad\
      0, 0, Gosu::Color::WHITE,
      width, 0, Gosu::Color::WHITE,
      0, height, Gosu::Color::WHITE,
      width, height, Gosu::Color::WHITE, 0
    
    args = DEST.map { |p| [p[0], p[1], Gosu::Color::WHITE] }.flatten + [0]
    if button_down? Gosu::KbTab then
      @image.draw_as_quad *args
    else
      @macro.draw_as_quad *args
    end
  end
end

FindMatrix.new.show
