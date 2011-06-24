$LOAD_PATH << '../lib'
require 'gosu'

Blob = Struct.new(:columns, :rows, :to_blob)

class ImageInsert < Gosu::Window
  def initialize
    super 800, 600, false
    @background = Gosu::Image.new(self, "media/Wallpaper.png", true)
    @cursor = Gosu::Image.new(self, "media/Cursor.png", false)
    @cursor_blob = Blob.new(@cursor.width, @cursor.height, @cursor.to_blob)
  end
  
  def needs_cursor?
    # nil works too now, yay
  end
  
  def draw
    draw_quad 0, 0, Gosu::Color::WHITE, width, 0, Gosu::Color::WHITE,
      0, height, Gosu::Color::RED, width, height, Gosu::Color::RED, 0
    @background.draw 10, 10, 0
    @cursor.draw mouse_x, mouse_y, 0
  end
  
  def button_down id
    case id
    when Gosu::MsLeft then
      @background.insert @cursor_blob, mouse_x.to_i - 10, mouse_y.to_i - 10
    when Gosu::KbP then
      @background.save File.expand_path("~/Desktop/Test.png")
    when Gosu::KbB then
      @background.save File.expand_path("~/Desktop/Test.bmp")
    end
  end
end

ImageInsert.new.show
