# This whole file is about backward compatibility.

# Define some helpers to deprecate code.
module Gosu
  DEPRECATION_STACKTRACE_LINES = 1

  # Adapted from RubyGems, but without the date part.
  def self.deprecate(klass, name, repl)
    klass.class_eval {
      old = "_deprecated_#{name}"
      alias_method old, name
      define_method name do |*args, &block|
        Gosu.deprecation_message(self, name, repl)
        send old, *args, &block
      end
    }
  end

  def self.deprecate_const(name, repl)
    send(:remove_const, name) if const_defined?(name)

    @@_deprecated_constants ||= {}
    @@_deprecated_constants[name] = repl
  end

  # Constant deprecation works by undefining the original constant and then re-adding it in
  # const_missing, so that each deprecation warning is only printed once.
  def self.const_missing(const_name)
    if @@_deprecated_constants && repl = @@_deprecated_constants[const_name]
      Gosu.deprecation_message(self, const_name, repl)
      const_get(repl)
    else
      super
    end
  end

  def self.deprecation_message(klass_or_full_message, name=nil, repl=nil)
    @@_deprecations_shown ||= {}

    msg = if klass_or_full_message.is_a?(String) and name.nil? and repl.nil?
      [ "DEPRECATION WARNING: #{klass_or_full_message},
        \n#Called from #{caller[1, DEPRECATION_STACKTRACE_LINES]}"
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

# No need to pass a Window to Image.
class Gosu::Image
  alias initialize_without_window initialize

  def initialize(*args)
    if args[0].is_a? Gosu::Window
      Gosu.deprecation_message("Passing a Window to Image#initialize has been deprecated in Gosu 0.9 and this method now uses an options hash, see https://www.libgosu.org/rdoc/Gosu/Image.html.")
      if args.size == 7
        initialize_without_window args[1], :tileable => args[2], :rect => args[3..-1]
      else
        initialize_without_window args[1], :tileable => args[2]
      end
    else
      initialize_without_window(*args)
    end
  end

  class << self
    alias from_text_without_window from_text
  end

  def self.from_text(*args)
    if args.size == 4
      Gosu.deprecation_message("Passing a Window to Image.from_text has been deprecated in Gosu 0.9 and this method now uses an options hash, see https://www.libgosu.org/rdoc/Gosu/Image.html.")
      from_text_without_window(args[1], args[3], :font => args[2])
    elsif args.size == 7
      Gosu.deprecation_message("Passing a Window to Image.from_text has been deprecated in Gosu 0.9 and this method now uses an options hash, see https://www.libgosu.org/rdoc/Gosu/Image.html.")
      from_text_without_window(args[1], args[3], :font => args[2],
        :spacing => args[4], :width => args[5], :align => args[6])
    else
      from_text_without_window(*args)
    end
  end
end

# No need to pass a Window to Sample and even better don't use Sample at all.
class Gosu::Sound
  alias initialize_without_window initialize

  def initialize(*args)
    if args.first.is_a? Gosu::Window
      args.shift
      Gosu.deprecation_message("Passing a Window to Sample#initialize has been deprecated in Gosu 0.7.17.")
    end
    initialize_without_window(*args)
  end

  def play_pan(pan, *args)
    Gosu.deprecation_message("Sound.play_pan has been deprecated in Gosu 0.15; use Sound.play with pan-Option instead, see https://www.libgosu.org/rdoc/Gosu/Sound.html")

    opts = { pan: pan }
    opts[:volume] = args.shift unless args.empty?
    opts[:speed] = args.shift unless args.empty?
    opts[:looping] = args.shift unless args.empty?
    play(opts)
  end
end

# No need to pass a Window to Song and even better don't use Song at all.
class Gosu::Song < Gosu::Sound
  alias initialize_without_window initialize

  def initialize(*args)
    Gosu.deprecation_message("Gosu::Song has been deprecated in Gosu 0.15; use Sound with streaming = true instead, see https://www.libgosu.org/rdoc/Gosu/Sound.html.")

    if args.first.is_a? Gosu::Window
      args.shift
      Gosu.deprecation_message("Passing a Window to Song#initialize has been deprecated in Gosu 0.7.17.")
    end

    @channel = nil
    new(*args, streaming: true)
  end

  def self.current_song
    # TODO: Not sure what to put here. Maybe something like @channel.finished? ? self : nil
  end

  def play(looping=true)
    @channel = super(looping: true)
  end

  def volume
    @channel.volume
  end

  def volume=(val)
    @channel.volume=val
  end

  def pause
    @channel.pause
  end

  def paused?
    @channel.paused?
  end

  def stop
    @channel.stop
  end

  def playing?
    @channel.playing?
  end
end

# Moved some Window-methods to the Gosu::Module
class Gosu::Window
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
end

# Constants
module Gosu
  Gosu.deprecate Window, :set_mouse_position, "Window#mouse_x= and Window#mouse_y="
  Gosu.deprecate Font, :draw_rot, "Font#draw with Gosu.rotate"
  
  # This was renamed because it's not actually a "copyright notice".
  # (https://en.wikipedia.org/wiki/Copyright_notice)
  deprecate_const :GOSU_COPYRIGHT_NOTICE, :LICENSES
  
  module Button; end
  
  # Channel was called SampleInstance before Gosu 0.13.0.
  deprecate_const :SampleInstance, :Channel
  deprecate_const :Sample, :Sound

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

    # Also import old-style constants into Gosu::Button.
    Gosu::Button.const_set old_name, Gosu.const_get(new_name)
  end
end
