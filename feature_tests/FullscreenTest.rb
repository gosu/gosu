$LOAD_PATH << '../lib'
require 'gosu'

#### Cases that HAVE to work ####

# Undersized case (should NOT distort on a widescreen display)
#WIDTH, HEIGHT = 640, 480
# OS X, 4:3: Works

# Undersized case (should NOT distort on 4:3 display)
WIDTH, HEIGHT = 640, 320
# OS X, 4:3: Clipping missing, reported resolution wrong

#### Cases that should work some day ####

# Unusual size
#WIDTH, HEIGHT = 600, 450

# Oversized case
#WIDTH, HEIGHT = 6400, 4800

# Rotated case (slightly weirdo)
#WIDTH, HEIGHT = 480, 640

# VERY undersized case? :D
#WIDTH, HEIGHT = 1, 1

class Test < Gosu::Window
  def initialize
    super WIDTH, HEIGHT, true
    @wp = Gosu::Image.new self, "media/Wallpaper.png", true
    @cursor = Gosu::Image.new self, "media/Cursor.png", false
    @font = Gosu::Font.new self, Gosu::default_font_name, 20
  end
  
  def draw
    draw_quad 0, 0, 0xffff0000, WIDTH, 0, 0xffff0000, 0, HEIGHT, 0xffff0000, WIDTH, HEIGHT, 0xffff0000, 0
    
    factor_x = factor_y = [1.0 / @wp.width * WIDTH, 1.0 / @wp.height * HEIGHT].max
    angle = 0
    if WIDTH < HEIGHT then
      angle = -90
      factor_x, factor_y = factor_y, factor_x
    end
    @wp.draw_rot WIDTH / 2, HEIGHT / 2, 0, angle, 0.5, 0.5, factor_x, factor_y, 0x80ffffff
    
    @font.draw "Want #{WIDTH} x #{HEIGHT} pixels", 10, 10, 0
    @font.draw "= get #{width} x #{height} pixels", 10, 60, 0
    
    @cursor.draw mouse_x, mouse_y, 0
  end

  def button_down(id)
    if id == Gosu::Button::KbEscape
      close
    end
  end
end

Test.new.show
