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

class Gosu::Image
  BlobHelper = Struct.new(:columns, :rows, :to_blob)
  
  def self.from_blob(width, height, rgba = "\0\0\0\0" * (width * height))
    self.new(BlobHelper.new(width, height, rgba))
  end
end

# Color constants.
# This is cleaner than having SWIG define them.
module Gosu
  class ImmutableColor < Color
    private :alpha=, :red=, :green=, :blue=, :hue=, :saturation=, :value=
  end

  class Color
    NONE    = Gosu::ImmutableColor.new(0x00_000000)
    BLACK   = Gosu::ImmutableColor.new(0xff_000000)
    GRAY    = Gosu::ImmutableColor.new(0xff_808080)
    WHITE   = Gosu::ImmutableColor.new(0xff_ffffff)
    AQUA    = Gosu::ImmutableColor.new(0xff_00ffff)
    RED     = Gosu::ImmutableColor.new(0xff_ff0000)
    GREEN   = Gosu::ImmutableColor.new(0xff_00ff00)
    BLUE    = Gosu::ImmutableColor.new(0xff_0000ff)
    YELLOW  = Gosu::ImmutableColor.new(0xff_ffff00)
    FUCHSIA = Gosu::ImmutableColor.new(0xff_ff00ff)
    CYAN    = Gosu::ImmutableColor.new(0xff_00ffff)

    alias_method :hash, :gl
    def eql?(other)
      gl == other.gl
    end
  end
end

class Gosu::Window
  # Call Thread.pass every tick, which may or may not be necessary for friendly co-existence with
  # Ruby's Thread class.

  alias_method :_tick, :tick

  def tick
    Thread.pass
    _tick
  end
end
