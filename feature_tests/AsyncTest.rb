$LOAD_PATH << '../lib'
require 'gosu'

module ZOrder
  Background, Player, Stars, UI = *0..3
end

class Player
  attr_reader :score

  def initialize(window)
    @image = Gosu::Image.new(window, "../examples/media/Starfighter.bmp", false)
    @beep = Gosu::Sample.new(window, "../examples/media/Beep.wav")
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
    @color = Gosu::Color.new(0x80000000)
    @color.red = rand(255 - 40) + 40
    @color.green = rand(255 - 40) + 40
    @color.blue = rand(255 - 40) + 40
    @x = rand * 640
    @y = rand * 480
  end

  def draw  
    img = @animation[Gosu::milliseconds / 100 % @animation.size]
    img.draw(@x - img.width / 2.0, @y - img.height / 2.0,
        ZOrder::Stars, 1, 1, @color, :add)
  end
end

class GameWindow < Gosu::Window
  def initialize
    super(640, 480, false, 1)
        
    @player = Player.new(self)
    @player.warp(320, 240)

    @star_anim = Gosu::Image::load_tiles(self, "../examples/media/Star.png", 25, 25, false)
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
    
    #if rand(100) < 4 and @stars.size < 25 then
    if @stars.size < 100 then
      @stars.push(Star.new(@star_anim))
    end
  end

  def draw
    self.caption = "Gosu Testing Zone @ #{Gosu::fps} FPS"
    
    @last_tick ||= 0
    if now - @last_tick > 50 then
      tv = Time.now
      puts "Jitter @ #{tv.hour}:#{tv.min}.#{tv.sec}"
    end
    @last_tick = now

    if @bg_loading and @bg_loading.has_value then
      @bg = @bg_loading.value
      @bg_loading = nil
    end
    
    @bg.draw(0, 0, ZOrder::Background) unless @bg.nil?
    @player.draw
    @stars.each { |star| star.draw }
    @font.draw("Score: #{@player.score}", 10, 10, ZOrder::UI, 1.0, 1.0, 0xffffff00)
  end

  def button_down(id)
    if id == Gosu::Button::KbF1 then
      @bg = Gosu::Image.new(self, "../examples/media/Space.png", true)
    end
    
    if id == Gosu::Button::KbF2 then
      @bg = nil
      @bg_loading = Gosu::Image.async_new(self, "../examples/media/Space.png")
      @bg_loading = Gosu::Image.async_new(self, "../examples/media/Space.png")
      @bg_loading = Gosu::Image.async_new(self, "../examples/media/Space.png")
      @bg_loading = Gosu::Image.async_new(self, "../examples/media/Space.png")
    end
    
    if id == Gosu::Button::KbEscape
      close
    end
  end
end

window = GameWindow.new
window.show
