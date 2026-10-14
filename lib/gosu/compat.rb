# This whole file is about backward compatibility.

# Define some helpers to deprecate code.
module Gosu
  DEPRECATION_STACKTRACE_LINES = 1

  # Adapted from RubyGems, but without the date part.
  def self.deprecate(klass, name, repl)
    klass.class_eval {
      old = "_deprecated_#{name}"
      alias_method old, name
      define_method name do |*args, **kwargs, &block|
        Gosu.deprecation_message(self, name, repl)
        send old, *args, **kwargs, &block
      end
    }
  end

  # The old C extension accepted options as a trailing Hash (`options = {}`).
  # -> Convert this to keyword arguments to support e.g. `super(640, 480, options)`.
  # Returns the args and kwargs to pass to the new method.
  def self.extract_legacy_options(args, kwargs, method_name)
    return args, kwargs unless kwargs.empty? and args.last.is_a?(Hash)

    deprecation_message("Passing options as a Hash to #{method_name} has been deprecated, use keyword arguments instead")
    return args[0...-1], args.last
  end

  def self.deprecation_message(klass_or_full_message, name = nil, repl = nil)
    @@_deprecations_shown ||= {}

    msg = if klass_or_full_message.is_a?(String) and name.nil? and repl.nil?
      [ "DEPRECATION WARNING: #{klass_or_full_message},
        \n#Called from #{caller.reject { |line| line.start_with?(__FILE__) }.first(DEPRECATION_STACKTRACE_LINES)}"
      ]
    else
      # Class method deprecation warnings result in something like this:
      # #<Class:Gosu::Window>::button_id_to_char is deprecated
      # Remove the instance-inspect stuff to make it look a bit better:
      if klass_or_full_message.kind_of?(Module)
        target = "#{klass_or_full_message.to_s.gsub(/#<Class:(.*)>/, '\1')}::"
      else
        "#{klass_or_full_message.class}#"
      end
      [ "DEPRECATION WARNING: #{target}#{name} is deprecated",
        repl == :none ? " with no replacement." : "; use #{repl} instead.",
        "\n#{target}#{name} called from #{Gosu.deprecation_caller.join("\n")}",
      ]
    end
    return if @@_deprecations_shown.has_key?(msg[0])
    @@_deprecations_shown[msg[0]] = true

    warn "#{msg.join}."
  end

  # This method removes the deprecation methods themselves from the stacktrace.
  def self.deprecation_caller
    caller.delete_if { |trace_line| trace_line =~ /(deprecat|const_missing)/ }
      .first(DEPRECATION_STACKTRACE_LINES)
  end
end

# These were useful for working with Direct3D and the Win32 API a long time ago,
# there was never a real reason to have them available in Ruby.
Gosu.deprecate Gosu::Color, :bgr, :none
Gosu.deprecate Gosu::Color, :abgr, :none

# No need to pass a Window to Image.
class Gosu::Image
  alias_method :initialize_without_window, :initialize

  def initialize(*args, **kwargs)
    args, kwargs = Gosu.extract_legacy_options(args, kwargs, "Image#initialize")

    if args[0].is_a? Gosu::Window
      # Old method signature: initialize(window, file_or_image, tileable = false, *rect)
      Gosu.deprecation_message("Passing a Window to Image#initialize has been deprecated in Gosu 0.9, and this method now uses named arguments. https://www.libgosu.org/rdoc/Gosu/Image.html")
      if args.size == 7
        initialize_without_window args[1], tileable: args[2], rect: args[3..-1], **kwargs
      elsif args.size == 3
        initialize_without_window args[1], tileable: args[2], **kwargs
      elsif args.size == 2
        initialize_without_window args[1], **kwargs
      else
        raise ArgumentError, "wrong number of arguments (given #{args.size})"
      end
    else
      initialize_without_window(*args, **kwargs)
    end
  end

  class << self
    alias_method :from_text_without_window, :from_text
    alias_method :from_markup_without_hash, :from_markup
    alias_method :load_tiles_without_window, :load_tiles
  end

  def self.from_text(*args, **kwargs)
    args, kwargs = Gosu.extract_legacy_options(args, kwargs, "Image.from_text")

    if args.first.is_a? Gosu::Window and args.size == 4
      # Old method signature: from_text(window, text, font_name, font_height)
      Gosu.deprecation_message("Passing a Window to Image.from_text has been deprecated in Gosu 0.9 and this method now uses named arguments. https://www.libgosu.org/rdoc/Gosu/Image.html")
      from_text_without_window(args[1], args[3], font: args[2])
    elsif args.first.is_a? Gosu::Window and args.size == 7
      # Old method signature: from_text(window, text, font_name, font_height, line_spacing, width, align)
      Gosu.deprecation_message("Passing a Window to Image.from_text has been deprecated in Gosu 0.9 and this method now uses named arguments. https://www.libgosu.org/rdoc/Gosu/Image.html")
      from_text_without_window(args[1], args[3],
                               font: args[2], spacing: args[4], width: args[5],
                               align: args[6])
    else
      from_text_without_window(*args, **kwargs)
    end
  end

  def self.from_markup(*args, **kwargs)
    args, kwargs = Gosu.extract_legacy_options(args, kwargs, "Image.from_markup")
    from_markup_without_hash(*args, **kwargs)
  end

  def self.load_tiles(*args, **kwargs)
    args, kwargs = Gosu.extract_legacy_options(args, kwargs, "Image.load_tiles")

    if args.first.is_a? Gosu::Window
      # Old method signature: load_tiles(window, file_or_image, w, h, tileable)
      Gosu.deprecation_message("Passing a Window to Image.load_tiles has been deprecated in Gosu 0.9, and this method now uses named arguments. https://www.libgosu.org/rdoc/Gosu/Image.html")
      load_tiles_without_window(args[1], args[2], args[3], tileable: args[4], **kwargs)
    else
      load_tiles_without_window(*args, **kwargs)
    end
  end
end

# No need to pass a Window to Sample.
class Gosu::Sample
  alias_method :initialize_without_window, :initialize

  def initialize(*args)
    if args.first.is_a? Gosu::Window
      args.shift
      Gosu.deprecation_message("Passing a Window to Sample#initialize has been deprecated in Gosu 0.7.17.")
    end
    initialize_without_window(*args)
  end
end

# No need to pass a Window to Song.
class Gosu::Song
  alias_method :initialize_without_window, :initialize

  def initialize(*args)
    if args.first.is_a? Gosu::Window
      args.shift
      Gosu.deprecation_message("Passing a Window to Song#initialize has been deprecated in Gosu 0.7.17.")
    end
    initialize_without_window(*args)
  end
end

# No need to pass a Window to Font.
class Gosu::Font
  alias_method :initialize_without_window, :initialize

  # Old method signature: Font.new(window, name, height)
  def initialize(*args, **kwargs)
    args, kwargs = Gosu.extract_legacy_options(args, kwargs, "Font#initialize")

    if args.first.is_a? Gosu::Window
      Gosu.deprecation_message("Passing a Window to Font#initialize has been deprecated in Gosu 0.9.")
      _, name, height = args
      initialize_without_window(height, name: name, **kwargs)
    else
      initialize_without_window(*args, **kwargs)
    end
  end

  alias_method :draw, :draw_markup
  Gosu.deprecate Gosu::Font, :draw, "Font#draw_text or Font#draw_markup"

  alias_method :draw_rel, :draw_markup_rel
  Gosu.deprecate Gosu::Font, :draw_rel, "Font#draw_text_rel or Font#draw_markup_rel"

  def draw_rot(markup, x, y, z, angle, scale_x = 1, scale_y = 1, c = 0xff_ffffff, mode = :default)
    Gosu.rotate(angle, x, y) { draw_markup(markup, x, y, z, scale_x, scale_y, c, mode) }
  end

  Gosu.deprecate Gosu::Font, :draw_rot, "Font#draw with Gosu.rotate"
end

class Gosu::Window
  alias_method :initialize_without_hash, :initialize

  # Without this, an options Hash would be interpreted as the positional `fullscreen` argument.
  def initialize(*args, **kwargs)
    args, kwargs = Gosu.extract_legacy_options(args, kwargs, "Window#initialize")
    initialize_without_hash(*args, **kwargs)
  end

  # Moved some Window methods to the Gosu module:
  # Class methods that have been turned into module methods.
  class << self
    def button_id_to_char(id)
      Gosu.button_id_to_char(id)
    end

    def char_to_button_id(ch)
      Gosu.char_to_button_id(ch)
    end
  end

  # Instance methods that have been turned into module methods.
  %w(draw_line draw_triangle draw_quad draw_rect
     flush gl clip_to record
     transform translate rotate scale
     button_id_to_char char_to_button_id button_down?).each do |method|
    define_method method.to_sym do |*args, &block|
      Gosu.send method, *args, &block
    end
  end

  def set_mouse_position(x, y)
    self.mouse_x = x
    self.mouse_y = y
  end

  Gosu.deprecate Gosu::Window, :set_mouse_position, "Window#mouse_x= and Window#mouse_y="
end

module Gosu
  alias_method :render_without_hash, :render
  module_function :render_without_hash

  module_function def render(*args, **kwargs, &block)
    args, kwargs = Gosu.extract_legacy_options(args, kwargs, "Gosu.render")
    render_without_hash(*args, **kwargs, &block)
  end
end

# Constants
module Gosu
  # This was renamed because it's not actually a "copyright notice".
  # (https://en.wikipedia.org/wiki/Copyright_notice)
  GOSU_COPYRIGHT_NOTICE = LICENSES
  deprecate_constant :GOSU_COPYRIGHT_NOTICE

  module Button; end

  # Channel was called SampleInstance before Gosu 0.13.0.
  SampleInstance = Channel
  deprecate_constant :SampleInstance

  # Support for KbLeft instead of KB_LEFT and Gp3Button2 instead of GP_3_BUTTON_2.
  Gosu.constants.grep(/^KB_|MS_|GP_/).each do |new_name|
    old_name = case new_name
      when :KB_ISO then "KbISO"
      when :KB_NUMPAD_PLUS then "KbNumpadAdd"
      when :KB_NUMPAD_MINUS then "KbNumpadSubtract"
      when :KB_EQUALS then "KbEqual"
      when :KB_LEFT_BRACKET then "KbBracketLeft"
      when :KB_RIGHT_BRACKET then "KbBracketRight"
      else new_name.to_s.capitalize.gsub(/_(.)/) { $1.upcase }
      end
    Gosu.const_set old_name, Gosu.const_get(new_name)
    deprecate_constant old_name

    # Also import old-style constants into Gosu::Button.
    Gosu::Button.const_set old_name, Gosu.const_get(new_name)
  end

  deprecate_constant :Button

  def self.language
    @language_cache ||= (user_languages.first || "en_US")
  end
end
