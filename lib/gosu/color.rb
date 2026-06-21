module Gosu
  class Color
    include Comparable

    # Gosu::Color is stored as an unsigned int in C
    # this is used for {Gosu::Image#to_blob} and {Gosu::Image.from_blob}
    SIZEOF = 4

    def self.argb(*args)
      Gosu::Color.new(*args)
    end

    def self.rgba(r, g, b, a)
      Gosu::Color.new(a, r, g, b)
    end

    def self.rgb(r, g, b)
      Gosu::Color.new(255, r, g, b)
    end

    def self.from_ahsv(alpha, h, s, v)
      __color = GosuFFI.Gosu_Color_create_from_ahsv(alpha.clamp(0.0, 255.0), h % 360.0, s.clamp(0.0, 1.0), v.clamp(0.0, 1.0))
      GosuFFI.check_last_error
      Gosu::Color.new(__color)
    end

    def self.from_hsv(h, s, v)
      __color = GosuFFI.Gosu_Color_create_from_hsv(h % 360.0, s.clamp(0.0, 1.0), v.clamp(0.0, 1.0))
      GosuFFI.check_last_error
      Gosu::Color.new(__color)
    end

    def initialize(*args)
      case args.size
      when 1
        @__color = GosuFFI.Gosu_Color_create(args.first)
        GosuFFI.check_last_error
      when 4
        @__color = GosuFFI.Gosu_Color_create_argb(args[0].clamp(0, 255), args[1].clamp(0, 255), args[2].clamp(0, 255), args[3].clamp(0, 255))
        GosuFFI.check_last_error
      else
        raise ArgumentError
      end
    end

    def alpha
      GosuFFI.check_last_error(GosuFFI.Gosu_Color_alpha(@__color))
    end

    def alpha=(value)
      @__color = GosuFFI.check_last_error(GosuFFI.Gosu_Color_set_alpha(@__color, value.clamp(0, 255)))
    end

    def red
      GosuFFI.check_last_error(GosuFFI.Gosu_Color_red(@__color))
    end

    def red=(value)
      @__color = GosuFFI.check_last_error(GosuFFI.Gosu_Color_set_red(@__color, value.clamp(0, 255)))
    end

    def green
      GosuFFI.check_last_error(GosuFFI.Gosu_Color_green(@__color))
    end

    def green=(value)
      @__color = GosuFFI.check_last_error(GosuFFI.Gosu_Color_set_green(@__color, value.clamp(0, 255)))
    end

    def blue
      GosuFFI.check_last_error(GosuFFI.Gosu_Color_blue(@__color))
    end

    def blue=(value)
      @__color = GosuFFI.check_last_error(GosuFFI.Gosu_Color_set_blue(@__color, value.clamp(0, 255)))
    end

    def value
      GosuFFI.check_last_error(GosuFFI.Gosu_Color_value(@__color))
    end

    def value=(value)
      @__color = GosuFFI.check_last_error(GosuFFI.Gosu_Color_set_value(@__color, value.clamp(0.0, 1.0)))
    end

    def saturation
      GosuFFI.check_last_error(GosuFFI.Gosu_Color_saturation(@__color))
    end

    def saturation=(value)
      @__color = GosuFFI.check_last_error(GosuFFI.Gosu_Color_set_saturation(@__color, value.clamp(0.0, 1.0)))
    end

    def hue
      GosuFFI.check_last_error(GosuFFI.Gosu_Color_hue(@__color))
    end

    def hue=(value)
      @__color = GosuFFI.check_last_error(GosuFFI.Gosu_Color_set_hue(@__color, value % 360.0))
    end

    def bgr
      GosuFFI.check_last_error(GosuFFI.Gosu_Color_bgr(@__color))
    end

    def abgr
      GosuFFI.check_last_error(GosuFFI.Gosu_Color_abgr(@__color))
    end

    def argb
      GosuFFI.check_last_error(GosuFFI.Gosu_Color_argb(@__color))
    end

    def gl
      @__color
    end

    def to_i
      @__color
    end

    def <=>(other)
      if other.is_a? Color
        to_i <=> other.to_i
      elsif other.is_a? Numeric
        to_i <=> other
      else
        nil
      end
    end

    alias hash gl

    def eql?(other)
      self == other
    end

    NONE = Gosu::Color.argb(0x00_000000)
    BLACK = Gosu::Color.argb(0xff_000000)
    GRAY = Gosu::Color.argb(0xff_808080)
    WHITE = Gosu::Color.argb(0xff_ffffff)
    AQUA = Gosu::Color.argb(0xff_00ffff)
    RED = Gosu::Color.argb(0xff_ff0000)
    GREEN = Gosu::Color.argb(0xff_00ff00)
    BLUE = Gosu::Color.argb(0xff_0000ff)
    YELLOW = Gosu::Color.argb(0xff_ffff00)
    FUCHSIA = Gosu::Color.argb(0xff_ff00ff)
    CYAN = Gosu::Color.argb(0xff_00ffff)
  end
end
