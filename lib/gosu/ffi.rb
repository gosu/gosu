module GosuFFI
  extend FFI::Library

  # Load gosu.{so,bundle,dylib} which was built by extconf.rb.
  # (Inspired by https://github.com/dbalatero/levenshtein-ffi/pull/11)
  # It might be useful to also allow loading Gosu installed as a system library, e.g.:
  # "/usr/local/lib/libgosu-ffi.dylib", # Homebrew on macOS (Intel) or manual installation
  # "/opt/homebrew/lib/libgosu-ffi.dylib", # Homebrew on macOS (Apple Silicon)
  # ...or built locally, for testing without having to re-install the gem:
  # File.expand_path("../../gosu/build/ffi/libgosu-ffi.so", __dir__),
  # File.expand_path("../../gosu/lib64/gosu-ffi.dll", __dir__),
  library = "gosu-ffi.#{RbConfig::MAKEFILE_CONFIG["DLEXT"]}"
  candidates = ["#{__dir__}/..", "#{__dir__}/../../ext/gosu-ffi"]
  candidates.prepend(Gem.loaded_specs["gosu"].extension_dir) if Gem.loaded_specs["gosu"]
  ffi_lib(candidates.map { |dir| File.expand_path(library, dir) })

  callback :_callback, [:pointer], :void
  callback :_callback_with_image, [:pointer, :pointer], :void
  callback :_callback_with_unsigned, [:pointer, :uint32], :void
  callback :_callback_with_string, [:pointer, :string], :void
  callback :_callback_returns_bool, [:pointer], :bool
  callback :_callback_hit_test_returns_unsigned, [:pointer, :int, :int], :uint32

  attach_function :Gosu_fps, [], :int
  attach_function :Gosu_flush, [], :void
  attach_function :Gosu_milliseconds, [], :long
  attach_function :Gosu_default_font_name, [], :string
  attach_function :Gosu_user_languages, [:_callback_with_string, :pointer], :void

  attach_function :Gosu_transform, [:double, :double, :double, :double, :double, :double, :double, :double,
                                    :double, :double, :double, :double, :double, :double, :double, :double,
                                    :_callback], :void
  attach_function :Gosu_translate, [:double, :double, :_callback], :void
  attach_function :Gosu_rotate, [:double, :double, :double, :_callback], :void
  attach_function :Gosu_scale, [:double, :double, :double, :double, :_callback], :void
  attach_function :Gosu_clip_to, [:double, :double, :double, :double, :_callback], :void

  attach_function :Gosu_gl_z, [:double, :_callback], :void
  attach_function :Gosu_gl, [:_callback], :void
  attach_function :Gosu_render, [:int, :int, :_callback, :uint32], :pointer
  attach_function :Gosu_record, [:int, :int, :_callback], :pointer

  attach_function :Gosu_button_down, [:uint32], :bool
  attach_function :Gosu_axis, [:uint32], :double
  attach_function :Gosu_button_id_to_char, [:uint32], :string
  attach_function :Gosu_button_char_to_id, [:string], :uint32
  attach_function :Gosu_button_name, [:uint32], :string
  attach_function :Gosu_gamepad_name, [:uint32], :string

  attach_function :Gosu_clipboard, [], :string
  attach_function :Gosu_set_clipboard, [:string], :void

  attach_function :Gosu_draw_line, [:double, :double, :uint32, :double, :double, :uint32, :double, :uint32], :void
  attach_function :Gosu_draw_triangle, [:double, :double, :uint32, :double, :double, :uint32,
                                        :double, :double, :uint32, :double, :uint32], :void
  attach_function :Gosu_draw_quad, [:double, :double, :uint32, :double, :double, :uint32,
                                    :double, :double, :uint32, :double, :double, :uint32, :double, :uint32], :void
  attach_function :Gosu_draw_rect, [:double, :double, :double, :double, :uint32, :double, :uint32], :void

  attach_function :Gosu_offset_x, [:double, :double], :double
  attach_function :Gosu_offset_y, [:double, :double], :double
  attach_function :Gosu_distance, [:double, :double, :double, :double], :double
  attach_function :Gosu_angle, [:double, :double, :double, :double], :double
  attach_function :Gosu_angle_diff, [:double, :double], :double
  attach_function :Gosu_random, [:double, :double], :double

  attach_function :Gosu_screen_width, [:pointer], :uint32
  attach_function :Gosu_screen_height, [:pointer], :uint32
  attach_function :Gosu_available_width, [:pointer], :uint32
  attach_function :Gosu_available_height, [:pointer], :uint32

  attach_function :Gosu_last_error, [], :string

  attach_function :Gosu_version, [], :string
  attach_function :Gosu_licenses, [], :string

  attach_function :Gosu_Channel_destroy, [:pointer], :void

  attach_function :Gosu_Channel_playing, [:pointer], :bool
  attach_function :Gosu_Channel_pause, [:pointer], :void
  attach_function :Gosu_Channel_paused, [:pointer], :bool
  attach_function :Gosu_Channel_resume, [:pointer], :void
  attach_function :Gosu_Channel_stop, [:pointer], :void
  attach_function :Gosu_Channel_set_volume, [:pointer, :double], :void
  attach_function :Gosu_Channel_set_speed, [:pointer, :double], :void
  attach_function :Gosu_Channel_set_pan, [:pointer, :double], :void

  attach_function :Gosu_Color_create, [:uint32], :uint32
  attach_function :Gosu_Color_create_argb, [:uchar, :uchar, :uchar, :uchar], :uint32
  attach_function :Gosu_Color_create_from_ahsv, [:uchar, :double, :double, :double], :uint32
  attach_function :Gosu_Color_create_from_hsv, [:double, :double, :double], :uint32

  attach_function :Gosu_Color_alpha, [:uint32], :uchar
  attach_function :Gosu_Color_red, [:uint32], :uchar
  attach_function :Gosu_Color_green, [:uint32], :uchar
  attach_function :Gosu_Color_blue, [:uint32], :uchar

  attach_function :Gosu_Color_set_alpha, [:uint32, :uchar], :uint32
  attach_function :Gosu_Color_set_red, [:uint32, :uchar], :uint32
  attach_function :Gosu_Color_set_green, [:uint32, :uchar], :uint32
  attach_function :Gosu_Color_set_blue, [:uint32, :uchar], :uint32

  attach_function :Gosu_Color_value, [:uint32], :double
  attach_function :Gosu_Color_saturation, [:uint32], :double
  attach_function :Gosu_Color_hue, [:uint32], :double

  attach_function :Gosu_Color_set_value, [:uint32, :double], :uint32
  attach_function :Gosu_Color_set_saturation, [:uint32, :double], :uint32
  attach_function :Gosu_Color_set_hue, [:uint32, :double], :uint32

  attach_function :Gosu_Color_bgr, [:uint32], :uint32
  attach_function :Gosu_Color_abgr, [:uint32], :uint32
  attach_function :Gosu_Color_argb, [:uint32], :uint32

  attach_function :Gosu_Font_create, [:int, :string, :uint32, :uint32], :pointer
  attach_function :Gosu_Font_destroy, [:pointer], :void

  attach_function :Gosu_Font_name, [:pointer], :string
  attach_function :Gosu_Font_height, [:pointer], :int

  attach_function :Gosu_Font_text_width, [:pointer, :string], :double
  attach_function :Gosu_Font_markup_width, [:pointer, :string], :double

  attach_function :Gosu_Font_draw_text, [:pointer, :string, :double, :double, :double,
                                         :double, :double, :uint32, :uint32], :void
  attach_function :Gosu_Font_draw_markup, [:pointer, :string, :double, :double, :double,
                                           :double, :double, :uint32, :uint32], :void

  attach_function :Gosu_Font_draw_text_rel, [:pointer, :string, :double, :double, :double,
                                             :double, :double, :double, :double, :uint32, :uint32], :void
  attach_function :Gosu_Font_draw_markup_rel, [:pointer, :string, :double, :double, :double,
                                               :double, :double, :double, :double, :uint32, :uint32], :void

  attach_function :Gosu_Font_set_image, [:pointer, :string, :uint32, :pointer], :void

  attach_function :Gosu_Image_create, [:string, :uint32], :pointer
  attach_function :Gosu_Image_destroy, [:pointer], :void

  attach_function :Gosu_Image_create_from_markup, [:string, :string, :double, :int, :double, :uint32, :uint32, :uint32], :pointer
  attach_function :Gosu_Image_create_from_text, [:string, :string, :double, :int, :double, :uint32, :uint32, :uint32], :pointer
  attach_function :Gosu_Image_create_from_blob, [:pointer, :ulong, :int, :int, :uint32], :pointer
  attach_function :Gosu_Image_create_from_subimage, [:pointer, :int, :int, :int, :int], :pointer
  attach_function :Gosu_Image_create_from_tiles, [:string, :int, :int, :_callback_with_image, :pointer, :uint32], :void
  attach_function :Gosu_Image_create_tiles_from_image, [:pointer, :int, :int, :_callback_with_image, :pointer, :uint32], :void

  attach_function :Gosu_Image_width, [:pointer], :int
  attach_function :Gosu_Image_height, [:pointer], :int

  attach_function :Gosu_Image_draw, [:pointer, :double, :double, :double, :double, :double, :uint32, :uint32], :void
  attach_function :Gosu_Image_draw_rot, [:pointer, :double, :double, :double, :double, :double, :double,
                                         :double, :double, :uint32, :uint32], :void
  attach_function :Gosu_Image_draw_as_quad, [:pointer, :double, :double, :uint32, :double, :double, :uint32,
                                             :double, :double, :uint32, :double, :double, :uint32, :double, :uint32], :void

  attach_function :Gosu_Image_save, [:pointer, :string], :void
  attach_function :Gosu_Image_to_blob, [:pointer], :pointer
  attach_function :Gosu_Image_insert, [:pointer, :pointer, :int, :int], :pointer

  attach_function :Gosu_Image_gl_tex_info_create, [:pointer], :pointer
  attach_function :Gosu_Image_gl_tex_info_destroy, [:pointer], :void
  attach_function :Gosu_Sample_create, [:string], :pointer
  attach_function :Gosu_Sample_destroy, [:pointer], :void

  attach_function :Gosu_Sample_play, [:pointer, :double, :double, :bool], :pointer
  attach_function :Gosu_Sample_play_pan, [:pointer, :double, :double, :double, :bool], :pointer

  attach_function :Gosu_Song_current_song, [], :pointer

  attach_function :Gosu_Song_create, [:string], :pointer
  attach_function :Gosu_Song_destroy, [:pointer], :void

  attach_function :Gosu_Song_play, [:pointer, :bool], :void
  attach_function :Gosu_Song_playing, [:pointer], :bool
  attach_function :Gosu_Song_pause, [:pointer], :void
  attach_function :Gosu_Song_paused, [:pointer], :bool
  attach_function :Gosu_Song_stop, [:pointer], :void

  attach_function :Gosu_Song_volume, [:pointer], :double
  attach_function :Gosu_Song_set_volume, [:pointer, :double], :void

  attach_function :Gosu_TextInput_create, [], :pointer
  attach_function :Gosu_TextInput_destroy, [:pointer], :void

  attach_function :Gosu_TextInput_caret_pos, [:pointer], :uint32
  attach_function :Gosu_TextInput_set_caret_pos, [:pointer, :uint32], :void
  attach_function :Gosu_TextInput_selection_start, [:pointer], :uint32
  attach_function :Gosu_TextInput_set_selection_start, [:pointer, :uint32], :void

  attach_function :Gosu_TextInput_text, [:pointer], :string
  attach_function :Gosu_TextInput_set_text, [:pointer, :string], :void
  attach_function :Gosu_TextInput_set_filter, [:pointer, :_callback_with_string, :pointer], :void
  attach_function :Gosu_TextInput_set_filter_result, [:pointer, :string], :void

  attach_function :Gosu_TextInput_insert_text, [:pointer, :string], :void
  attach_function :Gosu_TextInput_delete_backward, [:pointer], :void
  attach_function :Gosu_TextInput_delete_forward, [:pointer], :void

  attach_function :Gosu_Window_create, [:int, :int, :uint32, :double], :pointer
  attach_function :Gosu_Window_destroy, [:pointer], :void

  attach_function :Gosu_Window_set_draw, [:pointer, :_callback, :pointer], :void
  attach_function :Gosu_Window_set_update, [:pointer, :_callback, :pointer], :void
  attach_function :Gosu_Window_set_button_down, [:pointer, :_callback_with_unsigned, :pointer], :void
  attach_function :Gosu_Window_set_button_up, [:pointer, :_callback_with_unsigned, :pointer], :void
  attach_function :Gosu_Window_set_gamepad_connected, [:pointer, :_callback_with_unsigned, :pointer], :void
  attach_function :Gosu_Window_set_gamepad_disconnected, [:pointer, :_callback_with_unsigned, :pointer], :void
  attach_function :Gosu_Window_set_drop, [:pointer, :_callback_with_string, :pointer], :void
  attach_function :Gosu_Window_set_needs_redraw, [:pointer, :_callback_returns_bool, :pointer], :void
  attach_function :Gosu_Window_set_needs_cursor, [:pointer, :_callback_returns_bool, :pointer], :void
  attach_function :Gosu_Window_set_close, [:pointer, :_callback, :pointer], :void
  attach_function :Gosu_Window_set_gain_focus, [:pointer, :_callback, :pointer], :void
  attach_function :Gosu_Window_set_lose_focus, [:pointer, :_callback, :pointer], :void

  # Enable gosu's default button_down fullscreen toggle
  attach_function :Gosu_Window_default_button_down, [:pointer, :uint32], :void

  attach_function :Gosu_Window_show, [:pointer], :void
  attach_function :Gosu_Window_tick, [:pointer], :bool
  attach_function :Gosu_Window_close_immediately, [:pointer], :void

  attach_function :Gosu_Window_width, [:pointer], :int
  attach_function :Gosu_Window_set_width, [:pointer, :int], :void
  attach_function :Gosu_Window_height, [:pointer], :int
  attach_function :Gosu_Window_set_height, [:pointer, :int], :void

  attach_function :Gosu_Window_mouse_x, [:pointer], :double
  attach_function :Gosu_Window_set_mouse_x, [:pointer, :double], :void
  attach_function :Gosu_Window_mouse_y, [:pointer], :double
  attach_function :Gosu_Window_set_mouse_y, [:pointer, :double], :void

  attach_function :Gosu_Window_caption, [:pointer], :string
  attach_function :Gosu_Window_set_caption, [:pointer, :string], :void
  attach_function :Gosu_Window_update_interval, [:pointer], :double
  attach_function :Gosu_Window_set_update_interval, [:pointer, :double], :void
  attach_function :Gosu_Window_resize, [:pointer, :int, :int, :bool], :void
  attach_function :Gosu_Window_is_fullscreen, [:pointer], :bool
  attach_function :Gosu_Window_is_resizable, [:pointer], :bool
  attach_function :Gosu_Window_is_borderless, [:pointer], :bool
  attach_function :Gosu_Window_set_fullscreen, [:pointer, :bool], :void
  attach_function :Gosu_Window_set_resizable, [:pointer, :bool], :void
  attach_function :Gosu_Window_set_borderless, [:pointer, :bool], :void

  attach_function :Gosu_Window_sdl_window, [:pointer], :pointer

  attach_function :Gosu_Window_text_input, [:pointer], :pointer
  attach_function :Gosu_Window_set_text_input, [:pointer, :pointer], :void

  def self.color_to_uint32(color)
    color.is_a?(Gosu::Color) ? color.gl : color
  end

  def self.check_last_error(value = nil)
    if (err = Gosu_last_error())
      raise err
    end
    value
  end
end
