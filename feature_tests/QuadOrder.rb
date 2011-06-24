$LOAD_PATH << '../lib'
require 'gosu'

class Test < Gosu::Window
  def initialize
    super(800, 600, false)
    
    @font = Gosu::Font.new(self, Gosu::default_font_name, 20)
    @img = Gosu::Image.new(self, "media/WallpaperXXL.png", true)
    @vertices = [[0, 0], [400, 0], [0, 400], [500, 300]]
    @cur_vert = 0
  end
  
  def det a, b, p
    (b[0] - a[0]) * (p[1] - a[1]) - (p[0] - a[0]) * (b[1] - a[1])
  end
  
  def should_flip?
    #(det(*@vertices[0..2]) > 0) == (det(*@vertices[1..3]) > 0)
    false
  end
  
  def draw
    @vertices[@cur_vert] = [mouse_x, mouse_y]
    if not should_flip? then
      vertices = @vertices
    else
      vertices = [@vertices[0], @vertices[1], @vertices[3], @vertices[2]]
    end
    verts_with_colors = vertices.map { |coords| coords + [0xffffffff] }.flatten + [0]
    @img.draw_as_quad(*verts_with_colors)
    @vertices.each_with_index { |v, i| @font.draw "#{i+1}", v[0], v[1], 0 }
  end
  
  def button_down(id)
    @cur_vert = (@cur_vert + 1) % 4 if id == Gosu::MsLeft
    puts prod if id == Gosu::MsRight
    close if id == Gosu::KbEscape
  end
end

Test.new.show
