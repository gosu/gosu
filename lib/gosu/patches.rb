# Extend Numeric with simple angle conversion methods,
# for easier integration with Chipmunk.
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

# Backwards compatibility:
# Import constants into Gosu::Button.
module Gosu::Button
  Gosu.constants.each { |c| const_set(c, Gosu.const_get(c)) }
end

# Backwards compatibility:
# The old version of from_text has been deprecated in Gosu 0.9.
class Gosu::Image
  class << self
    alias from_text_without_window from_text
  end
  
  def self.from_text(*args)
    if args.size == 4
      from_text_without_window(args[1], args[3], :font => args[2])
    elsif args.size == 7
      from_text_without_window(args[1], args[3], :font => args[2],
        :spacing => args[4], :width => args[5], :align => args[6])
    else
      from_text_without_window(*args)
    end
  end
end

# Backwards compatibility:
# Passing a Window Sample#initialize has been deprecated in Gosu 0.7.17.
class Gosu::Sample
  alias initialize_without_window initialize
  
  def initialize(*args)
    args.shift if args.first.is_a? Gosu::Window
    initialize_without_window(*args)
  end
end

# Backwards compatibility:
# Passing a Window to Song#initialize has been deprecated in Gosu 0.7.17.
class Gosu::Song
  alias initialize_without_window initialize
  
  def initialize(*args)
    args.shift if args.first.is_a? Gosu::Window
    initialize_without_window(*args)
  end
end

# Color constants.
# This is cleaner than having SWIG define them.
module Gosu
  class ImmutableColor < Color
    private :alpha=, :red=, :green=, :blue=, :hue=, :saturation=, :value=
  end
  
  class Color
    NONE    = Gosu::ImmutableColor.new(0x00000000)
    BLACK   = Gosu::ImmutableColor.new(0xff000000)
    GRAY    = Gosu::ImmutableColor.new(0xff808080)
    WHITE   = Gosu::ImmutableColor.new(0xffffffff)
    AQUA    = Gosu::ImmutableColor.new(0xff00ffff)
    RED     = Gosu::ImmutableColor.new(0xffff0000)
    GREEN   = Gosu::ImmutableColor.new(0xff00ff00)
    BLUE    = Gosu::ImmutableColor.new(0xff0000ff)
    YELLOW  = Gosu::ImmutableColor.new(0xffffff00)
    FUCHSIA = Gosu::ImmutableColor.new(0xffff00ff)
    CYAN    = Gosu::ImmutableColor.new(0xff00ffff)
  end
end

# Backwards compatibility:
# Instance methods that have been turned into module methods.
class Gosu::Window
  def button_id_to_char(id)
    self.class.button_id_to_char(id)
  end
  
  def char_to_button_id(ch)
    self.class.char_to_button_id(ch)
  end
end

# Release OpenAL resources during Ruby's shutdown, not Gosu's.
at_exit do
  Gosu::Song.current_song.stop if Gosu::Song.current_song
  Gosu::_release_all_openal_resources
end
