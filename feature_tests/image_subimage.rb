# Encoding: UTF-8

require 'rubygems'
require 'gosu'

class ImageSubimageTest < Gosu::Window
  def initialize
    super 800, 600, false
    @background = Gosu::Image.new(self, "media/Wallpaper.png", true)
    @cursor = Gosu::Image.new(self, "media/Cursor.png", false)
  end
  
  def needs_cursor?
    # nil works too now, yay
  end
  
  def draw
    @background.draw 0, 0, 0
    @cursor.draw mouse_x, mouse_y, 0
  end
  
  def button_down id
    case id
    when Gosu::MsLeft then
      @cursor = @background.subimage mouse_x.to_i, mouse_y.to_i, 30, 30
    end
  end
end

ImageSubimageTest.new.show
