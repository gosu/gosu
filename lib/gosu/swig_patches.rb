# Exceptions in Window callbacks often get lost, this is especially annoying in draw/update.
# It is not clear whether this is a SWIG issue or if some stack frame is not exception
# compatible, but I just call protected_update etc. in the Ruby wrapper so I can add this
# custom debugging help:
class Gosu::Window
  alias_method :initialize_with_bitmask, :initialize

  def initialize(width, height, *args)
    $gosu_gl_blocks = nil

    if args.first.is_a? Hash
      flags = 0
      flags |= 1 if args.first[:fullscreen]
      flags |= 2 if args.first[:resizable]
      flags |= 4 if args.first[:borderless]
      initialize_with_bitmask(width, height, flags, args.first[:update_interval] || 16.666666)
    else
      initialize_with_bitmask(width, height, args[0] ? 1 : 0, args[1] || 16.666666)
    end
  end

  %w(update draw needs_redraw? needs_cursor?
     lose_focus button_down button_up
     gamepad_connected gamepad_disconnected drop).each do |callback|
    define_method "protected_#{callback}" do |*args|
      begin
        # If there has been an exception, don't do anything as to not make matters worse.
        # Conveniently turn the return value into a boolean result (for needs_cursor? etc).
        defined?(@_exception) ? false : !!send(callback, *args)
      rescue Exception => e
        # Exit the message loop naturally, then re-throw during the next tick.
        @_exception = e
        close!
        false
      end
    end
  end
  
  def protected_draw_2
    protected_draw
    $gosu_gl_blocks_2 = $gosu_gl_blocks
    $gosu_gl_blocks = nil
  end
  
  alias_method :show_internal, :show
  def show
    show_internal
    # Try to format the message nicely, without any useless patching that we are
    # doing here.
    if defined? @_exception
      if @_exception.backtrace.is_a? Array and not @_exception.backtrace.frozen?
        @_exception.backtrace.reject! { |line| line.include? "lib/gosu/swig_patches.rb" }
      end
      raise @_exception
    end
  end
end

module Gosu
  # Keep a reference to these blocks that is only cleared after Window#draw.
  # Otherwise, the GC might free these blocks while Gosu is still rendering.
  def self.gl(z = nil, &block)
    $gosu_gl_blocks ||= []
    $gosu_gl_blocks << block
    if z.nil?
      unsafe_gl(&block)
    else
      unsafe_gl(z, &block)
    end
  end
end

# SWIG somehow maps the instance method "argb" as an overload of the class
# method of the same name.
class Gosu::Color
  alias_method :argb, :to_i
end

# SWIG will not let me rename my method to '[]=', so use alias_method here.
class Gosu::Font
  alias_method :[]=, :set_image
end
