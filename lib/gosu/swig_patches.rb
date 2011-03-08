# SWIG workarounds
# These are offloaded into a separate file because rb_eval_string() is weird on Ruby 1.8.

# SWIG doesn't understand the C++ overloading.
class Gosu::Image
  def self.from_text(*args)
    args.size == 4 ? from_text4(*args) : from_text7(*args)
  end
end
