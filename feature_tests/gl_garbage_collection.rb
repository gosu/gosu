# Encoding: UTF-8

# Require cutting-edge development Gosu for testing.
$LOAD_PATH << '../lib'
require 'gosu'

# Window#gl(z) will enqueue a block of code to be run during Gosu's processing
# of the rendering queue. If the GC runs while the block is internally enqueued,
# Gosu must take care that the block can be discovered by the GC.

class GLGarbageCollectionTest < Gosu::Window
  def initialize
    super 400, 400, false
    @counter = 0
  end
  
  def draw
    10000.times do
      gl(42) { @counter += 1 }
    end
    GC.start
  end
  
  def update
    self.caption = @counter.to_s
  end
  
  def needs_cursor?
    true
  end
end

GLGarbageCollectionTest.new.show
