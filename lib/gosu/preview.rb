require 'gosu'

# Wrapper around Gosu 0.4-0.8 that provides the work-in-progress 0.9 interface

module Gosu
  class Font
    alias :initialize_old :initialize
    
    def initialize *args
      if args.first.is_a? Gosu::Window then
        initialize_old *args
      else
        height  = args[0]
        options = args[1] || {}
        name = options[:name] || Gosu::default_font_name
        initialize_old $window, name, height
      end
    end
  end
  
  class Window
    alias :initialize_old :initialize
    
    def initialize width, height, *args
      if args.empty? or args.first.is_a? Hash then
       options = args.first || {}
       fullscreen = !!options[:fullscreen]
       update_interval = options[:update_interval] || 16.66
      else
       fullscreen, update_interval = *args
      end
      $window = initialize_old width, height, fullscreen, update_interval
    end
  end
  
  class Image
    alias :initialize_old :initialize
    
    def initialize *args
      if args.first.is_a? Gosu::Window then
        initialize_old *args
      else
        source = args[0]
        tileable = !args[1] || args[1][:tileable]
        rect = args[1] && args[1][:rect]
        if rect then
          initialize_old $window, source, !!tileable, *rect
        else
          initialize_old $window, source, !!tileable
        end
      end
    end
    
    class <<self
      alias load_tiles_old load_tiles
    end
    
    def self.load_tiles *args
      if args.first.is_a? Gosu::Window then
        load_tiles_old *args
      else
        source = args[0]
        x, y = args[1..2]
        tileable = !args[3] || args[3][:tileable]
        load_tiles_old $window, source, x, y, !!tileable
      end
    end
    
    def self.from_text *args
      if args.first.is_a? Gosu::Window then
        args.size == 4 ? from_text4(*args) : from_text7(*args)
      else
        text      = args[0]
        height    = args[1]
        options   = args[2] || {}
        font      = options[:font]    || Gosu::default_font_name
        if width  = options[:width] then
          spacing = options[:spacing] || 0
          align   = options[:align]   || :left
          Gosu::Image.from_text7 $window, text, font, height, spacing, width, align
        else
          Gosu::Image.from_text4 $window, text, font, height
        end
      end
    end
  end
  
  def self.button_down? id
    $window.button_down? id
  end
  
  def self.mouse_x
    $window.mouse_x
  end
  
  def self.mouse_y
    $window.mouse_y
  end
  
  def self.draw_line *args
    $window.draw_line *args
  end
  
  def self.draw_triangle *args
    $window.draw_triangle *args
  end
  
  def self.draw_quad *args
    $window.draw_quad *args
  end
  
  def self.draw_rect x, y, width, height, color = 0xffffffff, z = 0, mode = :default
    $window.draw_quad x, y, color, x + width, y, color, x, y + height, color, x + width, y + height, color, z, mode
  end
  
  def self.clip_to *args, &draw
    $window.clip_to *args, &draw
  end
  
  def self.translate *args, &draw
    $window.translate *args, &draw
  end
  
  def self.scale *args, &draw
    $window.scale *args, &draw
  end
  
  def self.rotate *args, &draw
    $window.rotate *args, &draw
  end
  
  def self.transform *args, &draw
    $window.transform *args, &draw
  end
  
  def self.record width, height, &draw
    $window.record width, height, &draw
  end
  
  def self.gl *args, &draw
    $window.gl *args, &draw
  end
end
