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

    alias hash gl
    def eql?(other)
      gl == other.gl
    end
  end
end

class Gosu::Window
  # Call Thread.pass every tick, which may or may not be necessary for friendly co-existence with
  # Ruby's Thread class.

  alias _tick tick

  def tick
    Thread.pass
    _tick
  end
end

class Gosu::Image
  def ==(img)
    self.to_blob == img.to_blob
  end

  # Checks if two images are similar on a really basic level (check the difference of each channel)
  def similar_to?(img, treshold=0.90)
    return true if self == img
    return false unless img.is_a?(Gosu::Image)
    return false if self.width != img.width or self.height != img.height

    blob = img.to_blob
    differences = []

    self.to_blob.each_byte.with_index do |by,idx|
      delta = (by - blob.getbyte(idx)).abs
      differences << (delta / 255.0) if delta > 0
    end

    # If the average color difference is only subtle even on "large" parts of the image its still ok (e.g. differently rendered color gradients) OR
    # if the color difference is huge but on only a few pixels its ok too (e.g. a diagonal line may be off a few pixels)
    (1 - (differences.inject(:+) / differences.size) >= treshold) or (1 - (differences.size / blob.size.to_f) >= treshold)
  end
end

# Release OpenAL resources during Ruby's shutdown, not Gosu's.
at_exit do
  begin
    Gosu::Song.current_song.stop if Gosu::Song.current_song
    Gosu._release_all_openal_resources
  rescue
  end
end
