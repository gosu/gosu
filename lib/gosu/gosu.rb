module Gosu
  def self.fps
    GosuFFI.check_last_error(GosuFFI.Gosu_fps())
  end

  def self.flush
    GosuFFI.Gosu_flush()
    GosuFFI.check_last_error
  end

  def self.milliseconds
    GosuFFI.check_last_error(GosuFFI.Gosu_milliseconds())
  end

  def self.default_font_name
    GosuFFI.check_last_error(GosuFFI.Gosu_default_font_name())
  end

  def self.user_languages
    languages = []
    callback = proc { |data, string| languages << string if string }
    GosuFFI.Gosu_user_languages(callback, nil)
    GosuFFI.check_last_error(languages)
  end

  def self.gl(z = nil, &block)
    raise "Block not given!" unless block

    if z
      # When given a Z position, Gosu.gl will not call its block immediately, but enqueue it for later execution.
      # To prevent the block from getting GC'd, store it in a global variable which is cleared after Window.draw.
      $gosu_gl_blocks ||= []
      $gosu_gl_blocks << block

      GosuFFI.Gosu_gl_z(z, block)
    else
      GosuFFI.Gosu_gl(block)
    end

    GosuFFI.check_last_error
  end

  def self.render(width, height, retro: false, tileable: false, &block)
    # TODO: Exception translation from inner block
    __image = GosuFFI.Gosu_render(width, height, block, GosuFFI.image_flags(retro: retro, tileable: tileable))
    GosuFFI.check_last_error
    Gosu::Image.new(__image)
  end

  def self.record(width, height, &block)
    # TODO: Exception translation from inner block
    __image = GosuFFI.Gosu_record(width, height, block)
    GosuFFI.check_last_error
    Gosu::Image.new(__image)
  end

  def self.transform(e0, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15, &block)
    # TODO: Exception translation from inner block
    GosuFFI.Gosu_transform(e0, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15, block)
    GosuFFI.check_last_error
  end

  def self.translate(x, y, &block)
    # TODO: Exception translation from inner block
    GosuFFI.Gosu_translate(x, y, block)
    GosuFFI.check_last_error
  end

  def self.rotate(angle, around_x = 0, around_y = 0, &block)
    # TODO: Exception translation from inner block
    GosuFFI.Gosu_rotate(angle, around_x, around_y, block)
    GosuFFI.check_last_error
  end

  def self.scale(x, y = x, around_x = 0, around_y = 0, &block)
    # TODO: Exception translation from inner block
    GosuFFI.Gosu_scale(x, y, around_x, around_y, block)
    GosuFFI.check_last_error
  end

  # Note: On JRuby this seems to get "optimized out" for some reason
  # For now, call jruby with the `--dev` option
  def self.clip_to(x, y, width, height, &block)
    # TODO: Exception translation from inner block
    GosuFFI.Gosu_clip_to(x, y, width, height, block)
    GosuFFI.check_last_error
  end

  def self.button_down?(id)
    GosuFFI.check_last_error(GosuFFI.Gosu_button_down(id))
  end

  def self.axis(id)
    GosuFFI.check_last_error(GosuFFI.Gosu_axis(id))
  end

  def self.button_id_to_char(id)
    GosuFFI.check_last_error(GosuFFI.Gosu_button_id_to_char(id))
  end

  def self.char_to_button_id(id)
    GosuFFI.check_last_error(GosuFFI.Gosu_button_char_to_id(id))
  end

  def self.button_name(id)
    GosuFFI.check_last_error(GosuFFI.Gosu_button_name(id))
  end

  def self.gamepad_name(id)
    GosuFFI.check_last_error(GosuFFI.Gosu_gamepad_name(id))
  end

  def self.clipboard
    GosuFFI.check_last_error(GosuFFI.Gosu_clipboard())
  end

  def self.clipboard=(text)
    GosuFFI.Gosu_set_clipboard(text)
    GosuFFI.check_last_error
  end

  def self.draw_line(x1, y1, c1, x2, y2, c2, z = 0, mode = :default)
    GosuFFI.Gosu_draw_line(x1, y1, GosuFFI.color_to_uint32(c1), x2, y2, GosuFFI.color_to_uint32(c2), z, GosuFFI.blend_mode(mode))
    GosuFFI.check_last_error
  end

  def self.draw_triangle(x1, y1, c1, x2, y2, c2, x3, y3, c3, z = 0, mode = :default)
    GosuFFI.Gosu_draw_triangle(x1, y1, GosuFFI.color_to_uint32(c1), x2, y2, GosuFFI.color_to_uint32(c2),
                               x3, y3, GosuFFI.color_to_uint32(c3), z, GosuFFI.blend_mode(mode))
    GosuFFI.check_last_error
  end

  def self.draw_quad(x1, y1, c1, x2, y2, c2,
                     x3, y3, c3, x4, y4, c4,
                     z = 0, mode = :default)
    GosuFFI.Gosu_draw_quad(x1, y1, GosuFFI.color_to_uint32(c1), x2, y2, GosuFFI.color_to_uint32(c2),
                           x3, y3, GosuFFI.color_to_uint32(c3), x4, y4, GosuFFI.color_to_uint32(c4),
                           z, GosuFFI.blend_mode(mode))
    GosuFFI.check_last_error
  end

  def self.draw_rect(x, y, width, height, c, z = 0, mode = :default)
    GosuFFI.Gosu_draw_rect(x, y, width, height, GosuFFI.color_to_uint32(c), z, GosuFFI.blend_mode(mode))
    GosuFFI.check_last_error
  end

  def self.offset_x(angle, distance)
    GosuFFI.check_last_error(GosuFFI.Gosu_offset_x(angle, distance))
  end

  def self.offset_y(angle, distance)
    GosuFFI.check_last_error(GosuFFI.Gosu_offset_y(angle, distance))
  end

  def self.distance(x1, y1, x2, y2)
    GosuFFI.check_last_error(GosuFFI.Gosu_distance(x1, y1, x2, y2))
  end

  def self.angle(x1, y1, x2, y2)
    GosuFFI.check_last_error(GosuFFI.Gosu_angle(x1, y1, x2, y2))
  end

  def self.angle_diff(angle1, angle2)
    GosuFFI.check_last_error(GosuFFI.Gosu_angle_diff(angle1, angle2))
  end

  def self.random(min, max)
    GosuFFI.check_last_error(GosuFFI.Gosu_random(min, max))
  end

  def self.available_width(window = nil)
    if window.nil?
      GosuFFI.check_last_error(GosuFFI.Gosu_available_width(nil))
    else
      raise TypeError, "Instance of Gosu::Window or nil expected, got: #{window.class}" if not window.is_a? Gosu::Window
      GosuFFI.check_last_error(GosuFFI.Gosu_available_width(window.__pointer))
    end
  end

  def self.available_height(window = nil)
    if window.nil?
      GosuFFI.check_last_error(GosuFFI.Gosu_available_height(nil))
    else
      raise TypeError, "Instance of Gosu::Window or nil expected, got: #{window.class}" if not window.is_a? Gosu::Window
      GosuFFI.check_last_error(GosuFFI.Gosu_available_height(window.__pointer))
    end
  end

  def self.screen_width(window = nil)
    if window.nil?
      GosuFFI.check_last_error(GosuFFI.Gosu_screen_width(nil))
    else
      raise TypeError, "Instance of Gosu::Window or nil expected, got: #{window.class}" if not window.is_a? Gosu::Window
      GosuFFI.check_last_error(GosuFFI.Gosu_screen_width(window.__pointer))
    end
  end

  def self.screen_height(window = nil)
    if window.nil?
      GosuFFI.check_last_error(GosuFFI.Gosu_screen_height(nil))
    else
      raise TypeError, "Instance of Gosu::Window or nil expected, got: #{window.class}" if not window.is_a? Gosu::Window
      GosuFFI.check_last_error(GosuFFI.Gosu_screen_height(window.__pointer))
    end
  end

  def self.enable_undocumented_retrofication
    # TODO: Move implementation from C++ to Ruby.
  end
end
