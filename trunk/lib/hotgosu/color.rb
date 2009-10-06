module Gosu
  Color = ::ObjGosuColor
  
  class Color
    def self.new(*args)
      case args.length
      when 1 then
        alloc.initWithARGB args[0]
      when 4 then
        alloc.initWithAlpha args[0], red: args[1], green: args[2], blue: args[3]
      else
        raise ArgumentError.new("wrong number of arguments (#{args.length} for 1 or 4)")
      end
    end
  end
  
#   # Represents an ARGB color value with 8 bits for each channel. Can be
#   # replaced by literals of the form 0xaarrggbb in all of Gosu.
#   class Color
#     attr_accessor :hue, :saturation, :value
# 
#     # argb: 0xAARRGGBB integer.
#     def initialize(argb); end
#     # a:: Integer from 0..255
#     # r:: Integer from 0..255
#     # g:: Integer from 0..255
#     # b:: Integer from 0..255
#     def initialize(a, r, g, b); end
#     
#     # Converts a HSV triple into a color. Same as from_ahsv with alpha set to 255.
#     # h:: Integer from 0..360
#     # s:: Float from 0..1
#     # v:: Float from 0..1.
#     def self.from_hsv(h, s, v); end
# 
#     # Converts a HSV triple into a color, with a given alpha.
#     # a:: Integer from 0..255
#     # h:: Integer from 0..360
#     # s:: Float from 0..1
#     # v:: Float from 0..1.
#     def self.from_ahsv(a, h, s, v); end
#  end
end
