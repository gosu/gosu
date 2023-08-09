module Gosu
  class Font
    def initialize(height, name: Gosu.default_font_name, bold: false, italic: false, underline: false)
      __font = GosuFFI.Gosu_Font_create(height, name, GosuFFI.font_flags(bold, italic, underline))
      GosuFFI.check_last_error
      @memory_pointer = FFI::AutoPointer.new(__font, GosuFFI.method(:Gosu_Font_destroy))
    end

    def __pointer
      @memory_pointer
    end

    def name
      GosuFFI.check_last_error(GosuFFI.Gosu_Font_name(__pointer))
    end

    def height
      GosuFFI.check_last_error(GosuFFI.Gosu_Font_height(__pointer))
    end

    def text_width(text, scale_x = 1)
      GosuFFI.check_last_error(GosuFFI.Gosu_Font_text_width(__pointer, text.to_s)) * scale_x
    end

    def markup_width(text, scale_x = 1)
      GosuFFI.check_last_error(GosuFFI.Gosu_Font_markup_width(__pointer, text.to_s)) * scale_x
    end

    def draw_text(text, x, y, z, scale_x = 1, scale_y = 1, c = Gosu::Color::WHITE, mode = :default)
      GosuFFI.Gosu_Font_draw_text(__pointer, text.to_s, x, y, z, scale_x, scale_y, GosuFFI.color_to_uint32(c), GosuFFI.blend_mode(mode))
      GosuFFI.check_last_error
    end

    def draw_markup(text, x, y, z, scale_x = 1, scale_y = 1, c = Gosu::Color::WHITE, mode = :default)
      GosuFFI.Gosu_Font_draw_markup(__pointer, text.to_s, x, y, z, scale_x, scale_y, GosuFFI.color_to_uint32(c), GosuFFI.blend_mode(mode))
      GosuFFI.check_last_error
    end

    def draw_text_rel(text, x, y, z, rel_x, rel_y, scale_x = 1, scale_y = 1, c = Gosu::Color::WHITE, mode = :default)
      GosuFFI.Gosu_Font_draw_text_rel(__pointer, text.to_s, x, y, z, rel_x, rel_y, scale_x, scale_y, GosuFFI.color_to_uint32(c), GosuFFI.blend_mode(mode))
      GosuFFI.check_last_error
    end

    def draw_markup_rel(text, x, y, z, rel_x, rel_y, scale_x = 1, scale_y = 1, c = Gosu::Color::WHITE, mode = :default)
      GosuFFI.Gosu_Font_draw_markup_rel(__pointer, text.to_s, x, y, z, rel_x, rel_y, scale_x, scale_y, GosuFFI.color_to_uint32(c), GosuFFI.blend_mode(mode))
      GosuFFI.check_last_error
    end

    def []=(codepoint, image)
      GosuFFI.FF_COMBINATIONS.times do |font_flags|
        GosuFFI.Gosu_Font_set_image(__pointer, codepoint, font_flags, image.__pointer)
        GosuFFI.check_last_error
      end
    end
  end
end
