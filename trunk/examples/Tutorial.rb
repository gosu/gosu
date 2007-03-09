require 'gosu'

module ZOrder
  Background, Stars, Player, UI = *0..3
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

class Player
  def initialize(window)
    @image = Gosu::Image.new(window, "media/Starfighter.bmp", false)
    @beep = Gosu::Sample.new(window, "media/Beep.wav")
    @x = @y = @vel_x = @vel_y = @angle = 0.0
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
  
  def collect_stars(stars)
    stars.reject! do |star|
      dist_x = @x - star.x
      dist_y = @y - star.y
      dist = Math.sqrt(dist_x * dist_x + dist_y * dist_y)
      if dist < 35 then
        yield 10
        @beep.play
        true
      else
        false
      end
    end
  end

  def draw
    @image.draw_rot(@x, @y, ZOrder::Player, @angle)
  end
end
  
class GameWindow < Gosu::Window
  def initialize
    super(640, 480, false, 20)
    self.caption = "Gosu Tutorial Game"
    
    @font = Gosu::Font.new(self, Gosu::default_font_name, 20)
    @background_image = Gosu::Image.new(self, "media/Space.png", true)
    @star_anim = Gosu::Image::load_tiles(self, "media/Star.png", 25, 25, false)
    
    @player = Player.new(self)
    @player.warp(320, 240)
    @stars = Array.new
    @score = 0
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
    @player.collect_stars(@stars) { |gain| @score += gain }
    
    if rand(100) < 4 and @stars.size < 25 then
      @stars.push(Star.new(@star_anim))
    end
  end
  
  def button_down(id)
    if id == Gosu::Button::KbEscape
      close
    end
  end
  
  def draw
    @font.draw("Score: #{@score}", 10, 10, ZOrder::UI, 1.0, 1.0, 0xffffff00)
    @background_image.draw(0, 0, ZOrder::Background)
    @player.draw
    @stars.each { |star| star.draw }
  end
end

GameWindow.new.show
