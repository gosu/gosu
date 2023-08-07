require "ffi"

if RUBY_PLATFORM =~ /mswin$|mingw|win32\-|\-win32/
  binary_path = File.expand_path("../../gosu/lib", __dir__)#File.dirname(__FILE__)
  # 64-bit builds of Windows use "x64-mingw" as RUBY_PLATFORM
  binary_path += "64" if RUBY_PLATFORM =~ /^x64-/

  begin
    # Make DLLs available as shown here:
    # https://github.com/oneclick/rubyinstaller2/wiki/For-gem-developers
    require 'ruby_installer'
    RubyInstaller::Runtime.add_dll_directory(binary_path)
  rescue LoadError
    # Add this gem to the PATH on Windows so that bundled DLLs can be found.
    # When running through Ocra on Windows, we need to be careful to preserve the ENV["PATH"]
    # encoding (see #385).
    path_encoding = ENV["PATH"].encoding
    ENV["PATH"] = "#{binary_path.encode(path_encoding)};#{ENV["PATH"]}"
  end

  # Add the correct lib directory for the current version of Ruby (major.minor).
  $LOAD_PATH.unshift File.join(binary_path, RUBY_VERSION[/^\d+.\d+/])
end

module Gosu
  LIBRARY_PATH = [
    File.expand_path("../../gosu/build/ffi/libgosu-ffi.so", __dir__),
    File.expand_path("../../gosu/lib64/gosu-ffi.dll", __dir__),
    "/usr/local/lib/libgosu-ffi.dylib", # Homebrew on macOS (Intel) or manual installation
    "/opt/homebrew/lib/libgosu-ffi.dylib", # Homebrew on macOS (Apple Silicon)
    "gosu-ffi"
  ]

  extend FFI::Library
  ffi_lib Gosu::LIBRARY_PATH

  callback :_callback,             [:pointer],          :void
  callback :_callback_with_string, [:pointer, :string], :void

  attach_function :Gosu_fps,               [], :int
  attach_function :Gosu_flush,             [], :void
  attach_function :Gosu_milliseconds,      [], :long
  attach_function :Gosu_default_font_name, [], :string
  attach_function :Gosu_user_languages,    [:_callback_with_string, :pointer], :void

  attach_function :Gosu_transform, [:double, :double, :double, :double, :double, :double, :double, :double,
                                    :double, :double, :double, :double, :double, :double, :double, :double,
                                    :_callback], :void
  attach_function :Gosu_translate, [:double, :double, :_callback], :void
  attach_function :Gosu_rotate,    [:double, :double, :double, :_callback], :void
  attach_function :Gosu_scale,     [:double, :double, :double, :double, :_callback], :void
  attach_function :Gosu_clip_to,   [:double, :double, :double, :double, :_callback], :void

  attach_function :Gosu_gl_z,  [:double, :_callback],             :void
  attach_function :Gosu_gl,    [:_callback],                      :void
  attach_function :Gosu_render, [:int, :int, :_callback, :uint32], :pointer
  attach_function :Gosu_record, [:int, :int, :_callback],          :pointer

  attach_function :Gosu_button_down,       [:uint32], :bool
  attach_function :Gosu_axis,              [:uint32], :double
  attach_function :Gosu_button_id_to_char, [:uint32], :string
  attach_function :Gosu_button_char_to_id, [:string], :uint32
  attach_function :Gosu_button_name,       [:uint32], :string
  attach_function :Gosu_gamepad_name,      [:uint32], :string

  attach_function :Gosu_clipboard,      [], :string
  attach_function :Gosu_set_clipboard,  [:string], :void

  attach_function :Gosu_draw_line,     [:double, :double, :uint32, :double, :double, :uint32, :double, :uint32], :void
  attach_function :Gosu_draw_triangle, [:double, :double, :uint32, :double, :double, :uint32,
                                        :double, :double, :uint32, :double, :uint32],                            :void
  attach_function :Gosu_draw_quad,     [:double, :double, :uint32, :double, :double, :uint32,
                                        :double, :double, :uint32, :double, :double, :uint32, :double, :uint32], :void
  attach_function :Gosu_draw_rect,     [:double, :double, :double, :double, :uint32, :double, :uint32],          :void

  attach_function :Gosu_offset_x,   [:double, :double],                   :double
  attach_function :Gosu_offset_y,   [:double, :double],                   :double
  attach_function :Gosu_distance,   [:double, :double, :double, :double], :double
  attach_function :Gosu_angle,      [:double, :double, :double, :double], :double
  attach_function :Gosu_angle_diff, [:double, :double],                   :double
  attach_function :Gosu_random,     [:double, :double],                   :double

  attach_function :Gosu_screen_width,     [:pointer], :uint32
  attach_function :Gosu_screen_height,    [:pointer], :uint32
  attach_function :Gosu_available_width,  [:pointer], :uint32
  attach_function :Gosu_available_height, [:pointer], :uint32

  attach_function :Gosu_last_error, [], :string

  def self.fps
    Gosu_fps().tap { Gosu.check_last_error }
  end

  def self.flush
    Gosu_flush()
    Gosu.check_last_error
  end

  def self.milliseconds
    Gosu_milliseconds().tap { Gosu.check_last_error }
  end

  def self.default_font_name
    Gosu_default_font_name().tap { Gosu.check_last_error }
  end

  def self.user_languages
    languages = []
    callback = proc { |data, string| languages << string if string }
    Gosu_user_languages(callback, nil)
    Gosu.check_last_error
    languages
  end

  def self.gl(z = nil, &block)
    # Gosu.gl might not be called immediately
    # so to prevent block getting GC'd until used
    # store them in a global variable which is
    # cleared after rendering is complete
    # i.e. after Window.draw is done.
    $gosu_gl_blocks ||= []
    $gosu_gl_blocks << block

    raise "Block not given!" unless block

    if z
      Gosu_gl_z(z, block)
      Gosu.check_last_error
    else
      Gosu_gl(block)
      Gosu.check_last_error
    end
  end

  def self.render(width, height, retro: false, tileable: false, &block)
    # TODO: Exception translation from inner block
    __image = Gosu_render(width, height, block, Gosu.image_flags(retro: retro, tileable: tileable))
    Gosu.check_last_error
    Gosu::Image.new(__image)
  end

  def self.record(width, height, &block)
    # TODO: Exception translation from inner block
    __image = Gosu_record(width, height, block)
    Gosu.check_last_error
    Gosu::Image.new(__image)
  end

  def self.transform(e0, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15, &block)
    # TODO: Exception translation from inner block
    Gosu_transform(e0, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15, block)
    Gosu.check_last_error
  end

  def self.translate(x, y, &block)
    # TODO: Exception translation from inner block
    Gosu_translate(x, y, block)
    Gosu.check_last_error
  end

  def self.rotate(angle, around_x = 0, around_y = 0, &block)
    # TODO: Exception translation from inner block
    Gosu_rotate(angle, around_x, around_y, block)
    Gosu.check_last_error
  end

  def self.scale(x, y = x, around_x = 0, around_y = 0, &block)
    # TODO: Exception translation from inner block
    Gosu_scale(x, y, around_x, around_y, block)
    Gosu.check_last_error
  end

  # Note: On JRuby this seems to get "optimized out" for some reason
  # For now, call jruby with the `--dev` option
  def self.clip_to(x, y, width, height, &block)
    # TODO: Exception translation from inner block
    Gosu_clip_to(x, y, width, height, block)
    Gosu.check_last_error
  end

  def self.button_down?(id)
    Gosu_button_down(id).tap { Gosu.check_last_error }
  end

  def self.axis(id)
    Gosu_axis(id).tap { Gosu.check_last_error }
  end

  def self.button_id_to_char(id)
    Gosu_button_id_to_char(id).tap { Gosu.check_last_error }
  end

  def self.char_to_button_id(id)
    Gosu_button_char_to_id(id).tap { Gosu.check_last_error }
  end

  def self.button_name(id)
    name = Gosu_button_name(id).tap { Gosu.check_last_error }
    name&.empty? ? nil : name
  end

  def self.gamepad_name(id)
    name = Gosu_gamepad_name(id).tap { Gosu.check_last_error }
    name&.empty? ? nil : name
  end

  def self.clipboard
    Gosu_clipboard().tap { Gosu.check_last_error }
  end

  def self.clipboard=(text)
    Gosu_set_clipboard(text)
    Gosu.check_last_error
  end

  def self.draw_line(x1, y1, c1, x2, y2, c2, z = 0, mode = :default)
    Gosu_draw_line(x1, y1, color_to_drawop(c1), x2, y2, color_to_drawop(c2), z, Gosu.blend_mode(mode))
    Gosu.check_last_error
  end

  def self.draw_triangle(x1, y1, c1, x2, y2, c2, x3, y3, c3, z = 0, mode = :default)
    Gosu_draw_triangle(x1, y1, color_to_drawop(c1), x2, y2, color_to_drawop(c2),
                       x3, y3, color_to_drawop(c3), z, Gosu.blend_mode(mode))
    Gosu.check_last_error
  end

  def self.draw_quad(x1, y1, c1, x2, y2, c2,
                     x3, y3, c3, x4, y4, c4,
                     z = 0, mode = :default)
    Gosu_draw_quad(x1, y1, color_to_drawop(c1), x2, y2, color_to_drawop(c2),
                   x3, y3, color_to_drawop(c3), x4, y4, color_to_drawop(c4),
                   z, Gosu.blend_mode(mode))
    Gosu.check_last_error
  end

  def self.draw_rect(x, y, width, height, c, z = 0, mode = :default)
    Gosu_draw_rect(x, y, width, height, color_to_drawop(c), z, Gosu.blend_mode(mode))
    Gosu.check_last_error
  end

  def self.offset_x(angle, distance)
    Gosu_offset_x(angle, distance).tap { Gosu.check_last_error }
  end

  def self.offset_y(angle, distance)
    Gosu_offset_y(angle, distance).tap { Gosu.check_last_error }
  end

  def self.distance(x1, y1, x2, y2)
    Gosu_distance(x1, y1, x2, y2).tap { Gosu.check_last_error }
  end

  def self.angle(x1, y1, x2, y2)
    Gosu_angle(x1, y1, x2, y2).tap { Gosu.check_last_error }
  end

  def self.angle_diff(angle1, angle2)
    Gosu_angle_diff(angle1, angle2).tap { Gosu.check_last_error }
  end

  def self.random(min, max)
    Gosu_random(min, max).tap { Gosu.check_last_error }
  end

  def self.available_width(window = nil)
    raise TypeError, "Instance of Gosu::Window or nil expected, got: #{window.class}" if window && !window.is_a?(Gosu::Window)

    window = window.__pointer if window
    Gosu_available_width(window).tap { Gosu.check_last_error }
  end

  def self.available_height(window = nil)
    raise TypeError, "Instance of Gosu::Window or nil expected, got: #{window.class}" if window && !window.is_a?(Gosu::Window)

    window = window.__pointer if window
    Gosu_available_height(window).tap { Gosu.check_last_error }
  end

  def self.screen_width(window = nil)
    raise TypeError, "Instance of Gosu::Window or nil expected, got: #{window.class}" if window && !window.is_a?(Gosu::Window)

    window = window.__pointer if window
    Gosu_screen_width(window).tap { Gosu.check_last_error }
  end

  def self.screen_height(window = nil)
    raise TypeError, "Instance of Gosu::Window or nil expected, got: #{window.class}" if window && !window.is_a?(Gosu::Window)

    window = window.__pointer if window
    Gosu_screen_height(window).tap { Gosu.check_last_error }
  end

  def self.enable_undocumented_retrofication
    # TODO: Maybe hack-implement this? Easy enough - but should live in compat.rb
  end

  def self.color_to_drawop(color)
    color.is_a?(Gosu::Color) ? color.gl : color
  end

  def self.window_flags(fullscreen: false, resizable: false, borderless: false)
    flags = Gosu::WF_WINDOWED
    flags |= Gosu::WF_FULLSCREEN if fullscreen
    flags |= Gosu::WF_RESIZABLE if resizable
    flags |= Gosu::WF_BORDERLESS if borderless

    flags
  end

  # SEE: https://github.com/gosu/gosu/blob/master/Gosu/GraphicsBase.hpp
  def self.image_flags(retro: false, tileable: false)
    flags = 0

    flags |= Gosu::IF_RETRO if retro
    flags |= Gosu::IF_TILEABLE if tileable

    flags
  end

  def self.font_flags(bold, italic, underline)
    flags = 0
    flags |= Gosu::FF_BOLD if bold
    flags |= Gosu::FF_ITALIC if italic
    flags |= Gosu::FF_UNDERLINE if underline

    flags
  end

  def self.font_alignment_flags(flags)
    case flags
    when :left
      Gosu::AL_LEFT
    when :right
      Gosu::AL_RIGHT
    when :center
      Gosu::AL_CENTER
    when :justify
      Gosu::AL_JUSTIFY
    else
      return flags if flags.is_a?(Numeric)

      raise ArgumentError, "No such font alignment: #{flags}"
    end
  end

  def self.blend_mode(mode)
    case mode
    when :default
      Gosu::BM_DEFAULT
    when :additive, :add
      Gosu::BM_ADD
    when :multiply
      Gosu::BM_MULTIPLY
    else
      return mode if mode.is_a?(Numeric)

      raise ArgumentError, "No such blend mode: #{mode}"
    end
  end

  def self.check_last_error
    if (err = Gosu_last_error())
      raise err
    end
  end
end

# Individual classes need to be loaded after defining Gosu.check_last_error.

require_relative "gosu/numeric"

require_relative "gosu/channel"
require_relative "gosu/color"
require_relative "gosu/constants"
require_relative "gosu/font"
require_relative "gosu/gl_tex_info"
require_relative "gosu/image"
# require_relative "gosu/video"
require_relative "gosu/sample"
require_relative "gosu/song"
require_relative "gosu/text_input"
require_relative "gosu/version"
require_relative "gosu/window"

require_relative "gosu/compat"
