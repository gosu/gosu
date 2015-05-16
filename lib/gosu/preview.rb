require 'gosu'

# Wrapper around Gosu 0.4-0.8 that provides the work-in-progress 0.9 interface

module Gosu
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
