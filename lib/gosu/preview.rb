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
  
  def self.draw_rect x, y, width, height, color = 0xffffffff, z = 0, mode = :default
    Gosu::draw_quad x, y, color, x + width, y, color, x, y + height, color, x + width, y + height, color, z, mode
  end
end
