# SWIG workarounds
# These are offloaded into a separate file because rb_eval_string() is weird on Ruby 1.8.

# SWIG doesn't understand the C++ overloading.
class Gosu::Image
  def self.from_text(*args)
    args.size == 4 ? from_text4(*args) : from_text7(*args)
  end
end

# Linux workaround: instead of declaring a constant, we declare a hidden function and
# call it when we need to define the constant. Otherwise, we get a weird libGL.so segfault.
module Gosu
  def self.const_missing sym
    if sym == :MAX_TEXTURE_SIZE then
      const_set sym, __max_texture_size
    else
      super
    end
  end
end
