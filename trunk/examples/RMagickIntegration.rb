begin
  # In case you use Gosu via RubyGems.
  require 'rubygems'
rescue LoadError
  # In case you don't.
end

# A (too) simple Gorilla-style shooter for two players.
# Shows how Gosu and RMagick can be used together to generate a map, implement
# a dynamic landscape and generally look great.

# Known issues:
# * Collision detection of the missiles is lazy, allows shooting through thin walls.

require 'gosu'
require 'rmagick'

NULL_PIXEL = Magick::Pixel.from_color('none')

# The class for this game's map.
# Design:
# * Dynamic map creation at startup, holding it as RMagick Image in @image
# * Testing for solidity by testing @image's pixel values
# * Drawing by (re)creating an array of Gosu::Image instances, each representing
#   a part of the large @image
# * Blasting holes into the map is implemented by drawing and erasing portions
#   of @image, then setting the corresponding Gosu::Image instances to nil, so
#   they will be recreated in Map#draw

class Map
  WIDTH, HEIGHT = 800, 600
  TILE_SIZE = 100
  TILES_X = WIDTH / TILE_SIZE
  TILES_Y = HEIGHT / TILE_SIZE
  
  def initialize(window)
    # We'll need the window later for re-creating Gosu images.
    @window = window

    # Let's start with something simple and load the sky via RMagick.
    # Loading JPEG files isn't possible with Gosu, so say wow!
    sky = Magick::Image.read("media/Sky.jpg").first
    @sky = Gosu::Image.new(window, sky, true)
    
    # This is the one large RMagick image that represents the map.
    @image = Magick::Image.new(WIDTH, HEIGHT) { self.background_color = 'none' }
    
    # Set up a Draw object that fills with an earth texture.
    earth = Magick::Image.read('media/Earth.png').first.resize(1.5)
    gc = Magick::Draw.new
    gc.pattern('earth', 0, 0, earth.columns, earth.rows) { gc.composite(0, 0, 0, 0, earth) }    
    gc.fill('earth')
    gc.stroke('#603000').stroke_width(1.5)
    # Draw a smooth bezier island onto the map!
    polypoints = [0, HEIGHT]
    0.upto(TILES_X) do |x|
      polypoints += [x * TILE_SIZE, HEIGHT * 0.2 + rand(HEIGHT * 0.8)]
    end
    polypoints += [WIDTH, HEIGHT]
    gc.bezier(*polypoints)
    gc.draw(@image)
    
    # Create a bright-dark gradient fill, an image from it and change the map's
    # brightness with it.
    fill = Magick::GradientFill.new(0, HEIGHT * 0.4, WIDTH, HEIGHT * 0.4, '#fff', '#666')
    gradient = Magick::Image.new(WIDTH, HEIGHT, fill)
    gradient = @image.composite(gradient, 0, 0, Magick::InCompositeOp)
    @image.composite!(gradient, 0, 0, Magick::MultiplyCompositeOp)

    # Finally, place the star in the middle of the map, just onto the ground.
    star = Magick::Image.read('media/LargeStar.png').first
    star_y = 0
    star_y += 20 until solid?(WIDTH / 2, star_y)
    @image.composite!(star, (WIDTH - star.columns) / 2, star_y - star.rows * 0.85,
      Magick::DstOverCompositeOp)

    @gosu_images = [nil] * TILES_X * TILES_Y
  end
  
  def solid?(x, y)
    # Map is open at the top.
    return false if y < 0
    # Map is closed on all other sides.
    return true if x < 0 or x >= 800 or y >= 600
    # Inside of the map, determine solidity from the map image.
    @image.pixel_color(x, y) != NULL_PIXEL
  end
  
  def draw
    @sky.draw(0, 0, 0)
    TILES_Y.times do |y|
      TILES_X.times do |x|
        index = y * TILES_X + x
        if @gosu_images[index].nil? then
          part = @image.crop(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE)
          @gosu_images[index] = Gosu::Image.new(@window, part, true)
        end
        @gosu_images[index].draw(x * TILE_SIZE, y * TILE_SIZE, 0)
      end
    end
  end

  RADIUS = 25
  SH_RADIUS = 45
  
  def blast(x, y)
    left = (x - SH_RADIUS) / TILE_SIZE
    right = (x + SH_RADIUS) / TILE_SIZE
    top = (y - SH_RADIUS) / TILE_SIZE
    bottom = (y + SH_RADIUS) / TILE_SIZE
    
    @gosu_images[top * TILES_X + left] = nil
    @gosu_images[top * TILES_X + right] = nil
    @gosu_images[bottom * TILES_X + left] = nil
    @gosu_images[bottom * TILES_X + right] = nil
    
    if @crater_image.nil? then
      @crater_image = Magick::Image.new(2 * RADIUS, 2 * RADIUS) { self.background_color = 'none' }
      gc = Magick::Draw.new
      gc.fill('black').circle(RADIUS, RADIUS, RADIUS, 0)
      gc.draw(@crater_image)
      @crater_shadow = @crater_image.shadow(0, 0, (SH_RADIUS - RADIUS) / 2, 1)
    end
    
    @image.composite!(@crater_shadow, x - SH_RADIUS, y - SH_RADIUS, Magick::AtopCompositeOp)
    @image.composite!(@crater_shadow, x - SH_RADIUS, y - SH_RADIUS, Magick::AtopCompositeOp)
    @image.composite!(@crater_image, x - RADIUS, y - RADIUS, Magick::DstOutCompositeOp)
  end
end

class Player
  attr_reader :x, :y, :dead
  
  def initialize(window, x, y, color)
    @@images ||= Gosu::Image.load_tiles(window, "media/Soldier.png", 40, 50, false)
    @window, @x, @y, @color = window, x, y, color
    @vy = 0
    @dir = -1
    @angle = 90
  end
  
  def draw
    if dead then
      @@images[0].draw_rot(x, y, 0, 290 * @dir, 0.5, 0.65, @dir * 0.5, 0.5, @color)
      @@images[2].draw_rot(x, y, 0, 160 * @dir, 0.95, 0.5, 0.5, @dir * 0.5, @color)
    else
      if @show_walk_anim
        frame = Gosu::milliseconds / 200 % 2
      else
        frame = 0
      end
      
      @@images[frame].draw(x - 10 * @dir, y - 20, 0, @dir * 0.5, 0.5, @color)
      angle = @angle
      angle = 180 - angle if @dir == -1
      @@images[2].draw_rot(x, y - 5, 0, angle, 1, 0.5, 0.5, @dir * 0.5, @color)
    end
  end
  
  def update
    @show_walk_anim = false
    
    @vy += 1
    if @vy > 1 then
      @vy.times do
        if @window.map.solid?(x, y + 1)
          @vy = 0
          break
        else
          @y += 1
        end
      end
    else
      (-@vy).times do
        if @window.map.solid?(x, y - 15)
          @vy = 0
          break
        else
          @y -= 1
        end
      end
    end
    
    true
  end
  
  def aim_up
    @angle -= 2 unless @angle < 10
  end
  
  def aim_down
    @angle += 2 unless @angle > 170
  end
  
  def try_walk(dir)
    @show_walk_anim = true
    @dir = dir
    2.times { @y -= 1 unless @window.map.solid?(x, y - 15) }
    @x += dir unless @window.map.solid?(x + dir, y) or @window.map.solid?(x + dir, y - 14) 
    2.times { @y += 1 unless @window.map.solid?(x, y + 1) }
  end
  
  def try_jump
    @vy = -12 if @window.map.solid?(x, y + 1)
  end
  
  def shoot
    @window.objects << Missile.new(@window, x, y - 15, @angle * @dir)
  end
  
  def hit_by?(missile)
    if Gosu::distance(missile.x, missile.y, x, y) < 30 then
      @dead = true
      return true
    else
      return false
    end    
  end
end

class Missile
  attr_reader :x, :y, :vx, :vy
  
  def initialize(window, x, y, angle)
    @@explosion_sound ||= Gosu::Sample.new(window, "media/Explosion.wav")
    @vx, @vy = Gosu::offset_x(angle, 20).to_i, Gosu::offset_y(angle, 20).to_i
    @window, @x, @y = window, x + @vx, y + @vy
  end
  
  def update
    @x += @vx
    @y += @vy
    @vy += 1
    if @window.map.solid?(x, y) or @window.objects.any? { |o| o.hit_by?(self) } then
      5.times { @window.objects << Particle.new(@window, x - 25 + rand(51), y - 25 + rand(51)) }
      @window.map.blast(x, y)
      # Weeee, stereo sound!
      @@explosion_sound.play_pan((2 * @x - Map::WIDTH) / Map::WIDTH)
      return false
    else
      return true
    end
  end
  
  def draw
    @window.draw_quad(x-2, y-2, 0xff800000, x+2, y-2, 0xff800000, 
      x-2, y+2, 0xff800000, x+2, y+2, 0xff800000, 0)
  end
  
  def hit_by?(missile)
    false # can't be hit
  end
end

class Particle
  def initialize(window, x, y)
    @@image ||= Gosu::Image.new(window, 'media/Smoke.png', false)
    @x, @y = x, y
    @color = Gosu::Color.new(255, 255, 255, 255)
  end
  
  def update
    @y -= 5
    @x = @x - 1 + rand(3)
    @color.alpha -= 5
    @color.alpha > 0
  end
  
  def draw
    @@image.draw(@x - 25, @y - 25, 0, 1, 1, @color)
  end
  
  def hit_by?(missile)
    false
  end
end

class GameWindow < Gosu::Window
  attr_reader :map, :objects
  
  def initialize()
    super(800, 600, false)
    self.caption = "Medal of Anna - Gosu & RMagick integration demo"

    @player_instructions = []
    @player_won_messages = []
    2.times do |plr|
      @player_instructions << Gosu::Image.from_text(self,
        "It is the #{ plr == 0 ? 'green' : 'red' } toy soldier's turn.\n" +
        "(Arrow keys to walk and aim, Return to jump, Space to shoot)",
        Gosu::default_font_name, 25, 0, width, :center)
      @player_won_messages << Gosu::Image.from_text(self,
        "The #{ plr == 0 ? 'green' : 'red' } toy soldier has won!",
        Gosu::default_font_name, 25, 5, width, :center)
    end

    @map = Map.new(self)
    @players = [Player.new(self, 200, 40, 0xff308000), Player.new(self, 600, 40, 0xff803000)]
    @objects = @players.dup
    
    @current_player = 0
    @waiting = false
  end
  
  def draw
    @map.draw
    @objects.each { |o| o.draw }
    
    cur_text = @player_instructions[@current_player] if not @waiting
    cur_text = @player_won_messages[1 - @current_player] if @players[@current_player].dead
    
    if cur_text then
      x, y = 0, 30
      cur_text.draw(x - 1, y, 0, 1, 1, 0xff000000)
      cur_text.draw(x + 1, y, 0, 1, 1, 0xff000000)
      cur_text.draw(x, y - 1, 0, 1, 1, 0xff000000)
      cur_text.draw(x, y + 1, 0, 1, 1, 0xff000000)
      cur_text.draw(x, y, 0, 1, 1, 0xffffffff)
    end
  end
  
  def update
    @waiting &&= !@objects.grep(Missile).empty?
      
    @objects.reject! { |o| o.update == false }

    if not @waiting and not @players[@current_player].dead then
      player = @players[@current_player]
      player.aim_up if button_down? Gosu::KbUp
      player.aim_down if button_down? Gosu::KbDown
      player.try_walk(-1) if button_down? Gosu::KbLeft
      player.try_walk(1) if button_down? Gosu::KbRight
      player.try_jump if button_down? Gosu::KbReturn
    end
  end
  
  def button_down(id)
    if id == Gosu::KbSpace and not @waiting and not @players[@current_player].dead then
      @players[@current_player].shoot
      @current_player = 1 - @current_player
      @waiting = true
    end
    close if id == Gosu::KbEscape
  end
end

GameWindow.new.show
