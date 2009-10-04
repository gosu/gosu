module Gosu
  class Image < ::ObjGosuImage
    def self.new(window, filename, tileable = false)
      alloc.initWithWindow window, filename: filename, tileable: tileable
    end

    def self.new(window, filename_or_rmagick_image, tileable, src_x, src_y, src_width, src_height)
      raise :TODO
    end
    
    def draw(x, y, z, factor_x = 1, factor_y = 1, color = 0xffffffff, mode = :default)
      drawWithX x, y: y, z: z, factorX: factor_x, factorY: factor_y, color: color
      # TODO mode
    end

    def draw_rot(x, y, z, angle, center_x=0.5, center_y=0.5, factor_x=1, factor_y=1, color=0xffffffff, mode=:default)
    end
  end

#     # Like Window#draw_quad, but with this texture instead of colors. Can be used to implement advanced, non-rectangular drawing techniques and takes four points and the modulation color at each of them.
#     # The points can be in clockwise order, or in a Z shape, starting at the top left.
#     def draw_as_quad(x1, y1, c1, x2, y2, c2, x3, y3, c3, x4, y4, c4, z, mode=:default); end
# 
#     # Creates an Image containing a line of text.
#     #
#     # The text is always rendered in white. If you want to draw it in a
#     # different color, just modulate it by the target color.
#     # font_name:: Name of a system font, or a filename to a TTF file (must contain '/', does not work on Linux).
#     # font_height:: Height of the font in pixels.
#     def self.from_text(window, text, font_name, font_height); end
# 
#     # Creates an Image that is filled with the text given to the function.
#     #
#     # The text may contain line breaks.
#     #
#     # The text is always rendered in white. If you want to draw it in a
#     # different color, just modulate it by the target color.
#     # font_name:: Name of a system font, or a filename to a TTF file (must contain '/', does not work on Linux).
#     # font_height:: Height of the font in pixels.
#     # line_spacing:: Spacing between two lines of text in pixels.
#     # max_width:: Width of the bitmap that will be returned. Text will be split into multiple lines to avoid drawing over the right border. When a single word is too long, it will be truncated.
#     # align:: One of :left, :right, :center or :justify.
#     def self.from_text(window, text, font_name, font_height, line_spacing, max_width, align); end
#     
#     # Convenience function that splits a BMP or PNG file into an array
#     # of small rectangles and creates images from them.
#     # Returns the Array containing Image instances.
#     # tile_width:: If positive, specifies the width of one tile in pixels. If negative, the bitmap is divided into -tile_width rows.
#     # tile_height:: See tile_width.
#     def self.load_tiles(window, filename_or_rmagick_image, tile_width, tile_height, tileable); end
#     
#     # See examples/OpenGLIntegration.rb.
#     def gl_tex_info; end
#   end
# 
end
