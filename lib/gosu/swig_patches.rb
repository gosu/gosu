# SWIG workarounds
# These are offloaded into a separate file because rb_eval_string() is weird on Ruby 1.8.

# Exceptions in Window callbacks often get lost, this is especially annoying in draw/update.
# It is not clear whether this is a SWIG issue or if some stack frame is not exception
# compatible, but I just call update_ and draw_ in the Ruby wrapper so I can add this
# custom debugging help:
class Gosu::Window
  def update_
    update
  rescue Exception => e
    puts e.inspect
    puts e.backtrace
    raise e
  end

  def draw_
    draw
  rescue Exception => e
    puts e.inspect
    puts e.backtrace
    raise e
  end
end

# SWIG doesn't understand the C++ overloading.
class Gosu::Image
  def self.from_text(*args)
    args.size == 4 ? from_text4(*args) : from_text7(*args)
  end
end
