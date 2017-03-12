# Encoding: UTF-8
require_relative 'test_helper'

# Desired behavior of interactions between tick, show and close:
# - show() should present the window and start a new runloop, no matter what happened before.
# - Calling tick() for the first time, or after show() has returned, should always present the
#   window.
# - Calling close() from within the runloop started by show() should stop the runloop.
# - Calling close() between calls to tick() should make only the next call to tick() return false.

class TestWindow < Minitest::Test
  class ShyWindow < Gosu::Window
    def update
      close!
    end
  end
  
  def test_reopen
    window = ShyWindow.new(200, 150)
    assert !window.tick
    window.show
    assert !window.tick
  end
  
  class StubbornWindow < Gosu::Window
    def close
      # Do not call super
    end
    
    def button_down(id)
      close! if id == Gosu::KB_ESCAPE
    end
  end

  def test_close_callback
    window = StubbornWindow.new(200, 150)
    # window.show # <- for manual testing
    assert window.tick
    window.close
    assert window.tick
    window.close!
    assert !window.tick
    assert window.tick
  end
end
