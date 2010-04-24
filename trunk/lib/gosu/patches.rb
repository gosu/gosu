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
    
# Backwards compatibility: import the constants into Gosu::Button.
module Gosu::Button
  Gosu.constants.each { |c| const_set(c, Gosu.const_get(c)) }
end

# Backwards compatibility: Window arguments to Sample and Song
class Gosu::Sample
  alias new_initialize initialize
  
  def initialize(*args)
    args.shift if args.first.is_a? Gosu::Window
    new_initialize *args
  end
end
class Gosu::Song
  alias new_initialize initialize
  
  def initialize(*args)
    args.shift if args.first.is_a? Gosu::Window
    new_initialize *args
  end
end

# Color constants (SWIG messes up constants somehow)
class Gosu::Color
  NONE    = Gosu::Color.new(0x00000000)
  BLACK   = Gosu::Color.new(0xff000000)
  GRAY    = Gosu::Color.new(0xff808080)
  WHITE   = Gosu::Color.new(0xffffffff)            
  AQUA    = Gosu::Color.new(0xff00ffff)
  RED     = Gosu::Color.new(0xffff0000)
  GREEN   = Gosu::Color.new(0xff00ff00)
  BLUE    = Gosu::Color.new(0xff0000ff)
  YELLOW  = Gosu::Color.new(0xffffff00)
  FUCHSIA = Gosu::Color.new(0xffff00ff)
  CYAN    = Gosu::Color.new(0xff00ffff)
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