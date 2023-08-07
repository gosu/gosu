module Gosu
  class Image
    extend FFI::Library
    ffi_lib Gosu::LIBRARY_PATH

    callback :_callback_with_image, [:pointer, :pointer], :void

    attach_function :Gosu_Image_create,  [:string, :uint32], :pointer
    attach_function :Gosu_Image_destroy, [:pointer],         :void

    attach_function :Gosu_Image_create_from_markup,      [:string, :string, :double, :int, :double, :uint32, :uint32, :uint32], :pointer
    attach_function :Gosu_Image_create_from_text,        [:string, :string, :double, :int, :double, :uint32, :uint32, :uint32], :pointer
    attach_function :Gosu_Image_create_from_blob,        [:pointer, :ulong, :int, :int, :uint32],                               :pointer
    attach_function :Gosu_Image_create_from_subimage,    [:pointer, :int, :int, :int, :int],                                    :pointer
    attach_function :Gosu_Image_create_from_tiles,       [:string,  :int, :int, :_callback_with_image, :pointer, :uint32],      :void
    attach_function :Gosu_Image_create_tiles_from_image, [:pointer, :int, :int, :_callback_with_image, :pointer, :uint32],      :void

    attach_function :Gosu_Image_width,  [:pointer], :int
    attach_function :Gosu_Image_height, [:pointer], :int

    attach_function :Gosu_Image_draw,         [:pointer, :double, :double, :double, :double, :double, :uint32, :uint32], :void
    attach_function :Gosu_Image_draw_rot,     [:pointer, :double, :double, :double, :double, :double, :double,
                                               :double, :double, :uint32, :uint32],                                      :void
    attach_function :Gosu_Image_draw_as_quad, [:pointer, :double, :double, :uint32, :double, :double, :uint32,
                                               :double, :double, :uint32, :double, :double, :uint32, :double, :uint32],  :void

    attach_function :Gosu_Image_save,    [:pointer, :string],              :void
    attach_function :Gosu_Image_to_blob, [:pointer],                       :pointer
    attach_function :Gosu_Image_insert,  [:pointer, :pointer, :int, :int], :pointer

    attach_function :Gosu_Image_gl_tex_info_create,  [:pointer], :pointer
    attach_function :Gosu_Image_gl_tex_info_destroy, [:pointer], :void

    BlobHelper = Struct.new(:columns, :rows, :to_blob)

    def self.from_blob(width, height, rgba = "\0\0\0\0" * (width * height), retro: false, tileable: false)
      self.new(BlobHelper.new(width, height, rgba), retro: retro, tileable: tileable)
    end

    def self.from_text(markup, line_height, font: Gosu.default_font_name, width: -1, spacing: 0, align: :left,
                       bold: false, italic: false, underline: false, retro: false)
      __image = Gosu_Image_create_from_text(markup, font, line_height, width, spacing,
                                            Gosu.font_alignment_flags(align),
                                            Gosu.font_flags(bold, italic, underline),
                                            Gosu.image_flags(retro: retro))
      Gosu.check_last_error
      Gosu::Image.new(__image)
    end

    def self.from_markup(markup, line_height, font: Gosu.default_font_name, width: -1, spacing: 0, align: :left,
                       bold: false, italic: false, underline: false, retro: false)
      __image = Gosu_Image_create_from_markup(markup, font, line_height, width, spacing,
                                              Gosu.font_alignment_flags(align),
                                              Gosu.font_flags(bold, italic, underline),
                                              Gosu.image_flags(retro: retro))
      Gosu.check_last_error
      Gosu::Image.new(__image)
    end

    def self.load_tiles(filename_or_image, tile_width ,tile_height, retro: false, tileable: false)
      flags = Gosu.image_flags(retro: retro, tileable: tileable)

      images = []
      callback = proc { |data, image| images << Gosu::Image.new(image, retro: retro, tileable: tileable) }

      if filename_or_image.is_a?(String)
        Gosu_Image_create_from_tiles(filename_or_image, tile_width, tile_height, callback, nil, flags)
      else
        if filename_or_image.instance_of?(Gosu::Image)
          Gosu_Image_create_tiles_from_image(filename_or_image.__pointer, tile_width, tile_height, callback, nil, flags)
        else
          Gosu_Image_create_tiles_from_image(Gosu::Image.new(filename_or_image, retro: retro, tileable: tileable).__pointer, tile_width, tile_height, callback, nil, flags)
        end
      end

      Gosu.check_last_error

      return images
    end

    def initialize(object, retro: false, tileable: false)
      if object.is_a?(String)
        __image = Gosu_Image_create(object, Gosu.image_flags(retro: retro, tileable: tileable))
        Gosu.check_last_error
      elsif object.is_a?(FFI::Pointer)
        __image = object
      elsif object.respond_to?(:to_blob) &&
            object.respond_to?(:columns) &&
            object.respond_to?(:rows)

        blob_bytes = object.to_blob { self.format = 'RGBA'; self.depth = 8 }.bytes
        FFI::MemoryPointer.new(:uchar, blob_bytes.size) do |blob|
          blob.write_array_of_type(:uchar, :put_uchar, blob_bytes)
          __image = Gosu_Image_create_from_blob(blob, blob_bytes.size, object.columns, object.rows, Gosu.image_flags(retro: retro, tileable: tileable))
          Gosu.check_last_error
        end

        raise "Failed to load image from blob" if __image.null?
      else
        raise ArgumentError
      end

      @managed_pointer = FFI::AutoPointer.new(__image, Gosu::Image.method(:release))
    end

    def __pointer
      @managed_pointer
    end

    def width
      Gosu_Image_width(__pointer).tap { Gosu.check_last_error }
    end

    def height
      Gosu_Image_height(__pointer).tap { Gosu.check_last_error }
    end

    def draw(x, y, z, scale_x = 1, scale_y = 1, color = Gosu::Color::WHITE, flags = :default)
      Gosu_Image_draw(__pointer, x, y, z, scale_x, scale_y, Gosu.color_to_drawop(color), Gosu.blend_mode(flags))
      Gosu.check_last_error
    end

    def draw_rot(x, y, z, angle = 0, center_x = 0.5, center_y = 0.5, scale_x = 1, scale_y = 1, color = Gosu::Color::WHITE, flags = :default)
      Gosu_Image_draw_rot(__pointer, x, y, z, angle, center_x, center_y, scale_x, scale_y, Gosu.color_to_drawop(color), Gosu.blend_mode(flags))
      Gosu.check_last_error
    end

    def draw_as_quad(x1, y1, color1, x2, y2, color2, x3, y3, color3, x4, y4, color4, z = 0, mode = :default)
      Gosu_Image_draw_as_quad(x1, y1, Gosu.color_to_drawop(color1), x2, y2, Gosu.color_to_drawop(color2),
                              x3, y3, Gosu.color_to_drawop(color3), x4, y4, Gosu.color_to_drawop(color4),
                              z, Gosu.blend_mode(mode))
      Gosu.check_last_error
    end

    def save(filename)
      Gosu_Image_save(__pointer, filename)
      Gosu.check_last_error
    end

    def to_blob
      Gosu_Image_to_blob(__pointer).tap { Gosu.check_last_error }.read_string(width * height * Gosu::Color::SIZEOF)
    end

    def subimage(left, top, width, height)
      Gosu::Image.new(Gosu_Image_create_from_subimage(__pointer, left, top, width, height).tap { Gosu.check_last_error })
    end

    def insert(image, x, y)
      image_ = nil
      if image.is_a?(Gosu::Image)
        image_ = image.__pointer
      elsif image.respond_to?(:to_blob) &&
            image.respond_to?(:rows) &&
            image.respond_to?(:columns)
        image_ = Gosu::Image.new(image).__pointer
      else
        raise "Unable to insert image!"
      end

      Gosu_Image_insert(__pointer, image_, x, y)
      Gosu.check_last_error
    end

    def gl_tex_info
      tex_info = Gosu_Image_gl_tex_info_create(__pointer)
      tex_info ? GLTexInfo.new(tex_info) : nil
    end

    def self.release(pointer)
      Gosu::Image.Gosu_Image_destroy(pointer)
      Gosu.check_last_error
    end
  end
end
