require '../lib/gosu'

class Test < Gosu::Window
  def prod
    top_line = @vertices[0].zip(@vertices[1]).map { |a, b| a - b }
    bottom_line = @vertices[2].zip(@vertices[3]).map { |a, b| a - b }
    top_line[0] * bottom_line[1] - top_line[1] * bottom_line[0]
  end
  
  def initialize
    super(800, 600, false)
      
    @img = Gosu::Image.new(self, "media/Wallpaper.png", true)
    @vertices = [[0, 0], [400, 0], [0, 400], [500, 300]]
    @cur_vert = 0
  end

  def draw
    @vertices[@cur_vert] = [mouse_x, mouse_y]
    verts_with_colors = @vertices.map { |coords| coords + [0xffffffff] }.flatten + [0]
    @img.draw_as_quad(*verts_with_colors)
  end
  
  def button_down(id)
    @cur_vert = (@cur_vert + 1) % 4 if id == Gosu::MsLeft
    puts prod if id == Gosu::MsRight
    close if id == Gosu::KbEscape
  end
end

Test.new.show
