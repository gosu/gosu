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

class Gosu::Font
  # draw_text will stop parsing markup in Gosu 1.0.
  alias_method :draw_text, :draw_markup
  # draw_text_rel will stop parsing markup in Gosu 1.0.
  alias_method :draw_text_rel, :draw_markup_rel
  # text_width will stop parsing markup in Gosu 1.0.
  alias_method :text_width, :markup_width
end

class Gosu::Image
  # from_markup will stop parsing markup in Gosu 1.0.
  def self.from_markup(*args)
    self.from_text(*args)
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

# Release OpenAL resources during Ruby's shutdown, not Gosu's.
at_exit do
  begin
    Gosu::Song.current_song.stop if Gosu::Song.current_song
    Gosu._release_all_openal_resources
  rescue
  end
end
