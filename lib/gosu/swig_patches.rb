# SWIG workarounds
# These are offloaded into a separate file because rb_eval_string() is weird on Ruby 1.8.

# Exceptions in Window callbacks often get lost, this is especially annoying in draw/update.
# It is not clear whether this is a SWIG issue or if some stack frame is not exception
# compatible, but I just call protected_update etc. in the Ruby wrapper so I can add this
# custom debugging help:
class Gosu::Window
  %w(update draw needs_redraw? needs_cursor?
     lose_focus button_down button_up).each do |callback|
    define_method "protected_#{callback}" do |*args|
      begin
        # Turn into a boolean result for needs_cursor? etc while we are at it.
        # If there has been an exception, don't do anything as to not make matters worse.
        defined?(@_exception) ? false : !!send(callback, *args)
      rescue Exception => e
        # Exit the message loop naturally, then re-throw
        @_exception = e
        close
      end
    end
  end
  
  alias show_internal show
  def show
    show_internal
    # Try to format the message nicely, without any useless patching that we are
    # doing here.
    if defined? @_exception then
      if @_exception.backtrace.is_a? Array and not @_exception.backtrace.frozen? then
        @_exception.backtrace.reject! { |line| line.include? 'lib/gosu/swig_patches.rb' }
      end
      raise @_exception
    end
  end
end

# SWIG doesn't understand the C++ overloading, so we need this simple check in Ruby.
class Gosu::Image
  def self.from_text(*args)
    args.size == 4 ? from_text4(*args) : from_text7(*args)
  end
end
