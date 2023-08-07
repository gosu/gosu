module Gosu
  class Color
    extend FFI::Library
    ffi_lib Gosu::LIBRARY_PATH

    attach_function :Gosu_Color_create,           [:uint32],                           :uint32
    attach_function :Gosu_Color_create_argb,      [:uchar, :uchar, :uchar, :uchar],    :uint32
    attach_function :Gosu_Color_create_from_ahsv, [:uchar, :double, :double, :double], :uint32
    attach_function :Gosu_Color_create_from_hsv,  [:double, :double, :double],         :uint32

    attach_function :Gosu_Color_alpha,     [:uint32], :uchar
    attach_function :Gosu_Color_red,       [:uint32], :uchar
    attach_function :Gosu_Color_green,     [:uint32], :uchar
    attach_function :Gosu_Color_blue,      [:uint32], :uchar

    attach_function :Gosu_Color_set_alpha, [:uint32, :uchar], :uint32
    attach_function :Gosu_Color_set_red,   [:uint32, :uchar], :uint32
    attach_function :Gosu_Color_set_green, [:uint32, :uchar], :uint32
    attach_function :Gosu_Color_set_blue,  [:uint32, :uchar], :uint32

    attach_function :Gosu_Color_value,      [:uint32], :double
    attach_function :Gosu_Color_saturation, [:uint32], :double
    attach_function :Gosu_Color_hue,        [:uint32], :double

    attach_function :Gosu_Color_set_value,      [:uint32, :double], :uint32
    attach_function :Gosu_Color_set_saturation, [:uint32, :double], :uint32
    attach_function :Gosu_Color_set_hue,        [:uint32, :double], :uint32

    attach_function :Gosu_Color_bgr,  [:uint32], :uint32
    attach_function :Gosu_Color_abgr, [:uint32], :uint32
    attach_function :Gosu_Color_argb, [:uint32], :uint32

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
      __color = Gosu_Color_create_from_ahsv(alpha.clamp(0.0, 255.0), h % 360.0, s.clamp(0.0, 1.0), v.clamp(0.0, 1.0))
      Gosu.check_last_error
      Gosu::Color.new(__color)
    end

    def self.from_hsv(h, s, v)
      __color = Gosu_Color_create_from_hsv(h % 360.0, s.clamp(0.0, 1.0), v.clamp(0.0, 1.0))
      Gosu.check_last_error
      Gosu::Color.new(__color)
    end

    def initialize(*args)
      case args.size
      when 1
        @__color = Gosu_Color_create(args.first)
        Gosu.check_last_error
      when 4
        @__color = Gosu_Color_create_argb(args[0].clamp(0, 255), args[1].clamp(0, 255), args[2].clamp(0, 255), args[3].clamp(0, 255))
        Gosu.check_last_error
      else
        raise ArgumentError
      end
    end

    def alpha
      Gosu_Color_alpha(@__color).tap { Gosu.check_last_error }
    end

    def alpha=(value)
      @__color = Gosu_Color_set_alpha(@__color, value.clamp(0, 255)).tap { Gosu.check_last_error }
    end

    def red
      Gosu_Color_red(@__color).tap { Gosu.check_last_error }
    end

    def red=(value)
      @__color = Gosu_Color_set_red(@__color, value.clamp(0, 255)).tap { Gosu.check_last_error }
    end

    def green
      Gosu_Color_green(@__color).tap { Gosu.check_last_error }
    end

    def green=(value)
      @__color = Gosu_Color_set_green(@__color, value.clamp(0, 255)).tap { Gosu.check_last_error }
    end

    def blue
      Gosu_Color_blue(@__color).tap { Gosu.check_last_error }
    end

    def blue=(value)
      @__color = Gosu_Color_set_blue(@__color, value.clamp(0, 255)).tap { Gosu.check_last_error }
    end

    def value
      Gosu_Color_value(@__color).tap { Gosu.check_last_error }
    end

    def value=(value)
      @__color = Gosu_Color_set_value(@__color, value.clamp(0.0, 1.0)).tap { Gosu.check_last_error }
    end

    def saturation
      Gosu_Color_saturation(@__color).tap { Gosu.check_last_error }
    end

    def saturation=(value)
      @__color = Gosu_Color_set_saturation(@__color, value.clamp(0.0, 1.0)).tap { Gosu.check_last_error }
    end

    def hue
      Gosu_Color_hue(@__color).tap { Gosu.check_last_error }
    end

    def hue=(value)
      @__color = Gosu_Color_set_hue(@__color, value % 360.0).tap { Gosu.check_last_error }
    end

    def bgr
      Gosu_Color_bgr(@__color).tap { Gosu.check_last_error }
    end

    def abgr
      Gosu_Color_abgr(@__color).tap { Gosu.check_last_error }
    end

    def argb
      Gosu_Color_argb(@__color).tap { Gosu.check_last_error }
    end

    def gl
      @__color
    end

    def ==(other)
      gl == (other.is_a?(Numeric) ? other : other&.gl)
    end

    def <(other)
      gl < (other.is_a?(Numeric) ? other : other&.gl)
    end

    def >(other)
      gl > (other.is_a?(Numeric) ? other : other&.gl)
    end

    alias hash gl
    def eql?(other)
      self == other
    end

    NONE    = Gosu::Color.argb(0x00_000000)
    BLACK   = Gosu::Color.argb(0xff_000000)
    GRAY    = Gosu::Color.argb(0xff_808080)
    WHITE   = Gosu::Color.argb(0xff_ffffff)
    AQUA    = Gosu::Color.argb(0xff_00ffff)
    RED     = Gosu::Color.argb(0xff_ff0000)
    GREEN   = Gosu::Color.argb(0xff_00ff00)
    BLUE    = Gosu::Color.argb(0xff_0000ff)
    YELLOW  = Gosu::Color.argb(0xff_ffff00)
    FUCHSIA = Gosu::Color.argb(0xff_ff00ff)
    CYAN    = Gosu::Color.argb(0xff_00ffff)
  end
end
