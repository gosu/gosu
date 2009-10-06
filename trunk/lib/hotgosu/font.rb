module Gosu
  Font = ::ObjGosuFont
  
  class Font
    def self.new(window, font_name, height)
      alloc.initWithWindow window, fontName: font_name, height: height
    end

    def text_width(text, factor_x = 1)
      widthOfText(text) * factor_x
    end

    def draw(text, x, y, z, factor_x = 1, factor_y = 1, color = 0xffffffff, mode = :default)
      drawText text, atX: x, y: y, z: z, factorX: factor_x, factorY: factor_y, color: color
      # TODO mode
    end

    def draw_rel(text, x, y, z, rel_x, rel_y, factor_x = 1, factor_y = 1, color = 0xffffffff, mode = :default)
      raise :TODO
    end

    def draw_rot(text, x, y, z, angle, factor_x = 1, factor_y = 1, color = 0xffffffff, mode = :default)
      raise :TODO
    end
  end
end
