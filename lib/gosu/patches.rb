# Extend Numeric with simple angle conversion methods.
class ::Numeric
  def degrees_to_radians
    self * Math::PI / 180.0
  end
  def radians_to_degrees
    self * 180.0 / Math::PI
  end
  def gosu_to_radians
    (self - 90) * Math::PI / 180.0
  end
  def radians_to_gosu
    self * 180.0 / Math::PI + 90
  end
end
    
# Backwards compatibility: import constants into Gosu::Button.
module Gosu::Button
  Gosu.constants.each { |c| const_set(c, Gosu.const_get(c)) }
end

# Backwards compatibility: Window arguments to Sample and Song
class Gosu::Sample
  alias initialize_ initialize
  
  def initialize(*args)
    args.shift if args.first.is_a? Gosu::Window
    initialize_(*args)
  end
end
class Gosu::Song
  alias initialize_ initialize
  
  def initialize(*args)
    args.shift if args.first.is_a? Gosu::Window
    initialize_(*args)
  end
end

# Color constants (SWIG messes up constants somehow)
class Gosu::Color
  class Constant < Gosu::Color
  private
    def alpha=; end
    def red=; end
    def green=; end
    def blue=; end
    def hue=; end
    def saturation=; end
    def value=; end
  end
  
  NONE    = Gosu::Color::Constant.argb(0x00000000)
  BLACK   = Gosu::Color::Constant.argb(0xff000000)
  GRAY    = Gosu::Color::Constant.argb(0xff808080)
  WHITE   = Gosu::Color::Constant.argb(0xffffffff)            
  AQUA    = Gosu::Color::Constant.argb(0xff00ffff)
  RED     = Gosu::Color::Constant.argb(0xffff0000)
  GREEN   = Gosu::Color::Constant.argb(0xff00ff00)
  BLUE    = Gosu::Color::Constant.argb(0xff0000ff)
  YELLOW  = Gosu::Color::Constant.argb(0xffffff00)
  FUCHSIA = Gosu::Color::Constant.argb(0xffff00ff)
  CYAN    = Gosu::Color::Constant.argb(0xff00ffff)
end

# Instance methods for button_id_to_char and char_to_button_id
class Gosu::Window
  def button_id_to_char(id)
    self.class.button_id_to_char(id)
  end
  
  def char_to_button_id(ch)
    self.class.char_to_button_id(ch)
  end
end