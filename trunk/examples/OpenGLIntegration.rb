# use freshly built gosu
require '../gosu'
require 'rubygems'
require 'gl'
require 'glu'
include Gl
include Glu

module ZOrder
  Background, Stars, Player, UI = *0..3
end

class GLBackground
  POINTS_X = 5
  POINTS_Y = 5
  
  SCROLLS_PER_STEP = 30

  def initialize(window)
    @image = Gosu::Image.new(window, "media/Earth.png", true)
    @scrolls = 0
    @height_map = Array.new(POINTS_Y) { Array.new(POINTS_X) { rand } }
  end
  
  def scroll
    @scrolls += 1
    if @scrolls == SCROLLS_PER_STEP then
      @scrolls = 0
      @height_map.shift
      @height_map.push Array.new(POINTS_X) { rand }
    end
  end
  
  def exec_gl
    info = @image.gl_tex_info
    return unless info

    glDepthFunc(GL_GEQUAL)
    glEnable(GL_DEPTH_TEST)
    glEnable(GL_BLEND)

    glMatrixMode(GL_PROJECTION)
    glLoadIdentity
    glFrustum(-0.10, 0.10, -0.075, 0.075, 1, 100)

    glMatrixMode(GL_MODELVIEW)
    glLoadIdentity
    glTranslate(0, 0, -4)
  
    glEnable(GL_TEXTURE_2D)
    glBindTexture(GL_TEXTURE_2D, info.tex_name)
    
    offs_y = 1.0 * @scrolls / SCROLLS_PER_STEP
    
    0.upto(POINTS_Y - 2) do |y|
      0.upto(POINTS_X - 2) do |x|
        glBegin(GL_TRIANGLE_STRIP)
          z = @height_map[y][x]
          glColor4d(1, 1, 1, z)
          glTexCoord2d(info.left, info.top)
          glVertex3d(-0.5 + (x - 0.0) / (POINTS_X-1), -0.5 + (y - offs_y - 0.0) / (POINTS_Y-2), z)

          z = @height_map[y+1][x]
          glColor4d(1, 1, 1, z)
          glTexCoord2d(info.left, info.bottom)
          glVertex3d(-0.5 + (x - 0.0) / (POINTS_X-1), -0.5 + (y - offs_y + 1.0) / (POINTS_Y-2), z)
        
          z = @height_map[y][x + 1]
          glColor4d(1, 1, 1, z)
          glTexCoord2d(info.right, info.top)
          glVertex3d(-0.5 + (x + 1.0) / (POINTS_X-1), -0.5 + (y - offs_y - 0.0) / (POINTS_Y-2), z)

          z = @height_map[y+1][x + 1]
          glColor4d(1, 1, 1, z)
          glTexCoord2d(info.right, info.bottom)
          glVertex3d(-0.5 + (x + 1.0) / (POINTS_X-1), -0.5 + (y - offs_y + 1.0) / (POINTS_Y-2), z)
        glEnd
      end
    end
  end
end

class Player
  Speed = 7
  
  attr_reader :score

  def initialize(window, x, y)
    @image = Gosu::Image.new(window, "media/Starfighter.bmp", false)
    @beep = Gosu::Sample.new(window, "media/Beep.wav")
    @x, @y = x, y
    @score = 0
  end

  def move_left
    @x = [@x - Speed, 0].max
  end
  
  def move_right
    @x = [@x + Speed, 800].min
  end
  
  def accelerate
    @y = [@y - Speed, 50].max
  end
  
  def brake
    @y = [@y + Speed, 600].min
  end
  
  def draw
    @image.draw(@x - @image.width / 2, @y - @image.height / 2, ZOrder::Player)
  end
  
  def collect_stars(stars)
    stars.reject! do |star|
      if Gosu::distance(@x, @y, star.x, star.y) < 35 then
        @score += 10
        @beep.play
        true
      else
        false
      end
    end
  end
end

class Star
  attr_reader :x, :y
  
  def initialize(animation)
    @animation = animation
    @color = Gosu::Color.new(0xff000000)
    @color.red = rand(255 - 40) + 40
    @color.green = rand(255 - 40) + 40
    @color.blue = rand(255 - 40) + 40
    @x = rand * 800
    @y = 0
  end

  def draw  
    img = @animation[Gosu::milliseconds / 100 % @animation.size];
    img.draw_rot(@x, @y, ZOrder::Stars, @y, 0.5, 0.5, 1, 1, @color, :additive)
  end
  
  def update
    # Move towards bottom of screen
    @y += 3
    # Return false when out of screen (gets deleted then)
    @y < 650
  end
end

class GameWindow < Gosu::Window
  def initialize
    super(800, 600, false)
    self.caption = "Gosu & OpenGL Integration Demo"
    
    @gl_background = GLBackground.new(self)
    
    @player = Player.new(self, 400, 500)

    @star_anim = Gosu::Image::load_tiles(self, "media/Star.png", 25, 25, false)
    @stars = Array.new
    
    @font = Gosu::Font.new(self, Gosu::default_font_name, 20)
  end

  def update
    @player.move_left if button_down? Gosu::Button::KbLeft or button_down? Gosu::Button::GpLeft
    @player.move_right if button_down? Gosu::Button::KbRight or button_down? Gosu::Button::GpRight
    @player.accelerate if button_down? Gosu::Button::KbUp or button_down? Gosu::Button::GpUp
    @player.brake if button_down? Gosu::Button::KbDown or button_down? Gosu::Button::GpDown

    @player.collect_stars(@stars)
    
    @stars.reject! { |star| !star.update }
    
    @gl_background.scroll
    
    @stars.push(Star.new(@star_anim)) if rand(20) == 0
  end

  def draw
    gl do
      glClearColor(0.0, 0.2, 0.5, 1.0)
      glClearDepth(0)
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
     
      @gl_background.exec_gl
    end
    
    @player.draw
    @stars.each { |star| star.draw }
    @font.draw("Score: #{@player.score}", 10, 10, ZOrder::UI, 1.0, 1.0, 0xffffff00)
  end

  def button_down(id)
    if id == Gosu::Button::KbEscape
      close
    end
  end
end

window = GameWindow.new
window.show