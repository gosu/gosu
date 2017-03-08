# This whole file is about backward compatibility

# First - define some methods to tell the world about their old code

module Gosu
  DEPRECATION_STACKTRACE_LINES = 1

  # Basically copy&paste from Gem but remove the date-part.
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

  # The advantage of this approach is that the old Names won't show up in
  # any Codecompletion (like IRB) or *.constants, so you have to know the
  # exact old name to acutally use the old const.
  # However define the value after the first call, to show the warning only once.
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

    msg = if klass_or_full_message.is_a?(String) && name.nil? && repl.nil?
      [ "DEPRECATION WARNING: #{klass_or_full_message},
        \n#Called from #{caller[1, DEPRECATION_STACKTRACE_LINES]}"
      ]
    else
      # class-method deprecation warnings result in something like this: #<Class:Gosu::Window>::button_id_to_char is deprecated
      # so remove the instance-inspect stuff to make it look a bit better
      target = klass_or_full_message.kind_of?(Module) ? "#{klass_or_full_message.to_s.gsub(/#<Class:(.*)>/, '\1')}::" : "#{klass_or_full_message.class}#"
      [ "DEPRECATION WARNING: #{target}#{name} is deprecated",
        repl == :none ? " with no replacement." : "; use #{repl} instead.",
        "\n#{target}#{name} called from #{Gosu.deprecation_caller.join("\n")}",
      ]
    end
    return if @@_deprecations_shown.has_key?(msg[0])
    @@_deprecations_shown[msg[0]] = true

    warn "#{msg.join}."
  end

  # this method removes the noise of the caller due to the deprecation methods
  def self.deprecation_caller
    caller.delete_if { |trace_line| trace_line =~ /(deprecat|const_missing)/ }
      .first(DEPRECATION_STACKTRACE_LINES)
  end
end

# No need to pass a Window to Image, Sample and Song
class Gosu::Image
  alias initialize_without_window initialize

  def initialize(*args)
    if args[0].is_a? Gosu::Window
      Gosu.deprecation_message("Passing a Window to Image#initialize has been deprecated in Gosu 0.9 and the arguments have changed a bit as well. Check out https://www.libgosu.org/rdoc/Gosu/Image.html for more information.")
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
      Gosu.deprecation_message("Passing a Window to Image.from_text has been deprecated in Gosu 0.9 and the arguments have changed a bit as well. Check out https://www.libgosu.org/rdoc/Gosu/Image.html for more information.")
      from_text_without_window(args[1], args[3], :font => args[2])
    elsif args.size == 7
      Gosu.deprecation_message("Passing a Window to from_text has been deprecated in Gosu 0.9.")
      from_text_without_window(args[1], args[3], :font => args[2],
        :spacing => args[4], :width => args[5], :align => args[6])
    else
      from_text_without_window(*args)
    end
  end
end

class Gosu::Sample
  alias initialize_without_window initialize

  def initialize(*args)
    if args.first.is_a? Gosu::Window
      args.shift
      Gosu.deprecation_message("Passing a Window to Sample#initialize has been deprecated in Gosu 0.7.17.")
    end
    initialize_without_window(*args)
  end
end

class Gosu::Song
  alias initialize_without_window initialize

  def initialize(*args)
    if args.first.is_a? Gosu::Window
      args.shift
      Gosu.deprecation_message("Passing a Window to Song#initialize has been deprecated in Gosu 0.7.17.")
    end
    initialize_without_window(*args)
  end
end

# Moved some Window-methods to the Gosu::Module
class Gosu::Window
  
  # Class methods that have been turned into module methods.
  class << self
    def button_id_to_char(id)
      Gosu.button_id_to_char(id)
    end
    Gosu.deprecate self, :button_id_to_char, 'Gosu.button_id_to_char'

    def char_to_button_id(ch)
      Gosu.char_to_button_id(ch)
    end
    Gosu.deprecate self, :char_to_button_id, 'Gosu.char_to_button_id'
  end

  # Instance methods that have been turned into module methods.
  %w(draw_line draw_triangle draw_quad
     flush gl clip_to record
     transform translate rotate scale
     button_id_to_char char_to_button_id button_down?).each do |method|
    define_method method.to_sym do |*args, &block|
      Gosu.send method, *args, &block
    end
    Gosu.deprecate self, method, "Gosu.#{method}"
  end
end

# Constants
module Gosu
  # This was renamed, because it's not actually a "copyright notice" (Wikipedia: https://en.wikipedia.org/wiki/Copyright_notice).
  deprecate_const :GOSU_COPYRIGHT_NOTICE, :LICENSES

  module Button; end

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
    deprecate_const old_name.to_sym, new_name

    # Also import old-style constants into Gosu::Button.
    Gosu::Button.const_set old_name, Gosu.const_get(new_name)
  end
end
