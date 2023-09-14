module Gosu
  class Image
    def self.from_blob(width, height, rgba = "\0\0\0\0" * (width * height), retro: false, tileable: false)
      @struct ||= Struct.new(:columns, :rows, :to_blob)
      self.new(@struct.new(width, height, rgba), retro: retro, tileable: tileable)
    end

    def self.from_text(markup, line_height, font: Gosu.default_font_name, width: -1, spacing: 0, align: :left,
                                            bold: false, italic: false, underline: false, retro: false)
      __image = GosuFFI.Gosu_Image_create_from_text(markup, font, line_height, width, spacing,
                                                    GosuFFI.font_alignment_flags(align),
                                                    GosuFFI.font_flags(bold, italic, underline),
                                                    GosuFFI.image_flags(retro: retro))
      GosuFFI.check_last_error
      Gosu::Image.new(__image)
    end

    def self.from_markup(markup, line_height, font: Gosu.default_font_name, width: -1, spacing: 0, align: :left,
                                              bold: false, italic: false, underline: false, retro: false)
      __image = GosuFFI.Gosu_Image_create_from_markup(markup, font, line_height, width, spacing,
                                                      GosuFFI.font_alignment_flags(align),
                                                      GosuFFI.font_flags(bold, italic, underline),
                                                      GosuFFI.image_flags(retro: retro))
      GosuFFI.check_last_error
      Gosu::Image.new(__image)
    end

    def self.load_tiles(filename_or_image, tile_width, tile_height, retro: false, tileable: false)
      flags = GosuFFI.image_flags(retro: retro, tileable: tileable)

      images = []
      callback = proc { |data, image| images << Gosu::Image.new(image) }

      if filename_or_image.is_a? String
        GosuFFI.Gosu_Image_create_from_tiles(filename_or_image, tile_width, tile_height, callback, nil, flags)
      else
        if filename_or_image.is_a? Gosu::Image
          GosuFFI.Gosu_Image_create_tiles_from_image(filename_or_image.__pointer, tile_width, tile_height, callback, nil, flags)
        else
          image = Gosu::Image.new(filename_or_image, retro: retro, tileable: tileable)
          GosuFFI.Gosu_Image_create_tiles_from_image(image.__pointer, tile_width, tile_height, callback, nil, flags)
        end
      end

      GosuFFI.check_last_error

      return images
    end

    def initialize(object, retro: false, tileable: false, rect: nil)
      if rect and rect.size != 4
        raise ArgumentError, "Expected 4-element array as rect"
      end

      flags = GosuFFI.image_flags(retro: retro, tileable: tileable)

      if object.is_a? String
        if rect
          __image = GosuFFI.Gosu_Image_create_from_rect(object, *rect, flags)
        else
          __image = GosuFFI.Gosu_Image_create(object, flags)
        end
        GosuFFI.check_last_error
      elsif object.is_a?(FFI::Pointer)
        __image = object
      elsif object.respond_to?(:to_blob) and
            object.respond_to?(:columns) and
            object.respond_to?(:rows)
      
        blob = object.to_blob { self.format = "RGBA"; self.depth = 8 }

        # This creates a copy of the Ruby string data, which shouldn't be necessary with CRuby.
        ptr = FFI::MemoryPointer.from_string(blob)
        rect ||= [0, 0, object.columns, object.rows]
        # Do not consider the terminating null byte part of the ptr length.
        __image = GosuFFI.Gosu_Image_create_from_blob(ptr, ptr.size - 1, object.columns, object.rows, *rect, flags)
        ptr.free

        GosuFFI.check_last_error
      else
        raise ArgumentError, "Expected String or RMagick::Image (or a type compatible with it)"
      end

      @managed_pointer = FFI::AutoPointer.new(__image, GosuFFI.method(:Gosu_Image_destroy))
    end

    def __pointer
      @managed_pointer
    end

    def width
      GosuFFI.check_last_error(GosuFFI.Gosu_Image_width(__pointer))
    end

    def height
      GosuFFI.check_last_error(GosuFFI.Gosu_Image_height(__pointer))
    end

    def draw(x, y, z, scale_x = 1, scale_y = 1, color = Gosu::Color::WHITE, flags = :default)
      GosuFFI.Gosu_Image_draw(__pointer, x, y, z, scale_x, scale_y, GosuFFI.color_to_uint32(color), GosuFFI.blend_mode(flags))
      GosuFFI.check_last_error
    end

    def draw_rot(x, y, z, angle = 0, center_x = 0.5, center_y = 0.5, scale_x = 1, scale_y = 1, color = Gosu::Color::WHITE, flags = :default)
      GosuFFI.Gosu_Image_draw_rot(__pointer, x, y, z, angle, center_x, center_y, scale_x, scale_y, GosuFFI.color_to_uint32(color), GosuFFI.blend_mode(flags))
      GosuFFI.check_last_error
    end

    def draw_as_quad(x1, y1, color1, x2, y2, color2, x3, y3, color3, x4, y4, color4, z = 0, mode = :default)
      GosuFFI.Gosu_Image_draw_as_quad(x1, y1, GosuFFI.color_to_uint32(color1), x2, y2, GosuFFI.color_to_uint32(color2),
                                      x3, y3, GosuFFI.color_to_uint32(color3), x4, y4, GosuFFI.color_to_uint32(color4),
                                      z, GosuFFI.blend_mode(mode))
      GosuFFI.check_last_error
    end

    def save(filename)
      GosuFFI.Gosu_Image_save(__pointer, filename)
      GosuFFI.check_last_error
    end

    def to_blob
      blob = GosuFFI.check_last_error(GosuFFI.Gosu_Image_to_blob(__pointer))
      blob.read_string(width * height * Gosu::Color::SIZEOF)
    end

    def subimage(left, top, width, height)
      __subimage = GosuFFI.Gosu_Image_create_from_subimage(__pointer, left, top, width, height)
      GosuFFI.check_last_error
      Gosu::Image.new(__subimage)
    end

    def insert(image, x, y)
      image_ = nil
      if image.is_a?(Gosu::Image)
        image_ = image.__pointer
      elsif image.respond_to?(:to_blob) and
            image.respond_to?(:rows) and
            image.respond_to?(:columns)
        image_ = Gosu::Image.new(image).__pointer
      else
        raise "Unable to insert image!"
      end

      GosuFFI.Gosu_Image_insert(__pointer, image_, x, y)
      GosuFFI.check_last_error
    end

    def gl_tex_info
      tex_info = GosuFFI.check_last_error(GosuFFI.Gosu_Image_gl_tex_info_create(__pointer))
      tex_info ? GLTexInfo.new(tex_info) : nil
    end
  end
end
