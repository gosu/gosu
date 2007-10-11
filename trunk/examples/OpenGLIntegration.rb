# use freshly builded gosu
require '../gosu'
require 'rubygems'
require 'gl'
require 'glu'
include Gl
include Glu

module ZOrder
  Background, Stars, Player, UI = *0..3
end

class Player
  attr_reader :score

  def initialize(window)
    @image = Gosu::Image.new(window, "media/Starfighter.bmp", false)
    @beep = Gosu::Sample.new(window, "media/Beep.wav")
    @x = @y = @vel_x = @vel_y = @angle = 0.0
    @score = 0
  end

  def warp(x, y)
    @x, @y = x, y
  end
  
  def turn_left
    @angle -= 4.5
  end
  
  def turn_right
    @angle += 4.5
  end
  
  def accelerate
    @vel_x += Gosu::offset_x(@angle, 0.5)
    @vel_y += Gosu::offset_y(@angle, 0.5)
  end
  
  def move
    @x += @vel_x
    @y += @vel_y
    @x %= 640
    @y %= 480
    
    @vel_x *= 0.95
    @vel_y *= 0.95
  end

  def draw
    @image.draw_rot(@x, @y, ZOrder::Player, @angle)
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
    @x = rand * 640
    @y = rand * 480
  end

  def draw  
    img = @animation[Gosu::milliseconds / 100 % @animation.size];
    img.draw(@x - img.width / 2.0, @y - img.height / 2.0,
        ZOrder::Stars, 1, 1, @color, :additive)
  end
end

class GameWindow < Gosu::Window
  def initialize
    super(640, 480, false, 0)
    self.caption = "Hackish OpenGL Integration"
    
    @background_image = Gosu::Image.new(self, "media/Space.png", true)
    
    @player = Player.new(self)
    @player.warp(320, 240)

    @star_anim = Gosu::Image::load_tiles(self, "media/Star.png", 25, 25, false)
    @stars = Array.new
    
    @font = Gosu::Font.new(self, Gosu::default_font_name, 20)
  end

  def update
    if button_down? Gosu::Button::KbLeft or button_down? Gosu::Button::GpLeft then
      @player.turn_left
    end
    if button_down? Gosu::Button::KbRight or button_down? Gosu::Button::GpRight then
      @player.turn_right
    end
    if button_down? Gosu::Button::KbUp or button_down? Gosu::Button::GpButton0 then
      @player.accelerate
    end
    @player.move
    @player.collect_stars(@stars)
    
    if @stars.size < 25 then
      @stars.push(Star.new(@star_anim))
    end
  end

  def custom_draw
    $pyramid_angle ||= 0
    $pyramid_angle += 0.2

    glClearDepth(1.0)
    glClear(GL_DEPTH_BUFFER_BIT)

    glDepthFunc(GL_LEQUAL)
    glEnable(GL_DEPTH_TEST)
    glEnable(GL_CULL_FACE)
    glEnable(GL_BLEND)
    glMatrixMode(GL_PROJECTION)
    glLoadIdentity
    # Calculate aspect ratio of the window
    gluPerspective(45.0, 1.0 * width / height, 0.1, 100.0)
    glMatrixMode(GL_MODELVIEW)
    glLoadIdentity

    # Move left 1.5 units and into the screen 6.0 units
    glTranslatef(0, 0, -4.0)

    # Rotate the pyramid on the Y-axis
    glRotatef($pyramid_angle, 0.0, 1.0, 0.0)
    # Draw a pyramid
    glBegin(GL_POLYGON)
        # Draw front side of pyramid
        glColor4f(1.0, 0.0, 0.0, 1.0)
        glVertex3f( 0.0,  1.0, 0.0)
        
        glColor4f(0.0, 1.0, 0.0, 0.5)
        glVertex3f(-1.0, -1.0, 1.0)
        
        glColor4f(0.0, 0.0, 1.0, 0.5)
        glVertex3f(1.0, -1.0, 1.0)

        # Draw right side of pyramid
        glColor4f(1.0, 0.0, 0.0, 1.0)
        glVertex3f( 0.0,  1.0, 0.0)
        
        glColor4f(0.0, 0.0, 1.0, 0.5)
        glVertex3f( 1.0, -1.0, 1.0)
        
        glColor4f(0.0, 1.0, 0.0, 0.5)
        glVertex3f(1.0, -1.0, -1.0)

        # Draw back side of pyramid
        glColor4f(1.0, 0.0, 0.0, 1.0)
        glVertex3f( 0.0,  1.0, 0.0)
        
        glColor4f(0.0, 1.0, 0.0, 0.5)
        glVertex3f(1.0, -1.0, -1.0)
        
        glColor4f(0.0, 0.0, 1.0, 0.5)
        glVertex3f(-1.0, -1.0, -1.0)

        # Draw left side of pyramid
        glColor4f(1.0, 0.0, 0.0, 1.0)
        glVertex3f( 0.0,  1.0, 0.0)
        
        glColor4f(0.0, 0.0, 1.0, 0.5)
        glVertex3f(-1.0, -1.0, -1.0)
        
        glColor4f(0.0, 1.0, 0.0, 0.5)
        glVertex3f(-1.0, -1.0, 1.0)
    glEnd
    
    glDisable(GL_CULL_FACE)
    glMatrixMode(GL_MODELVIEW)
    glLoadIdentity
    glMatrixMode(GL_PROJECTION)
    glLoadIdentity
    glOrtho(0, width, height, 0, -1, 1)
  end

  def draw
    @cur_second ||= Gosu::milliseconds / 1000
    @accum_frames ||= 0
    
    @accum_frames += 1
    if @cur_second != Gosu::milliseconds / 1000 then
      @cur_second = Gosu::milliseconds / 1000
      self.caption = "Hackish OpenGL Integration @ #{@accum_frames} FPS"
      @accum_frames = 0
    end
    
    @background_image.draw(0, 0, ZOrder::Background)
    
    flush

    custom_draw

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