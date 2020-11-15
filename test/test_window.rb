# Encoding: UTF-8
require_relative "test_helper"

# Desired behavior of interactions between tick, show and close:
# - show() should present the window and start a new runloop, no matter what happened before.
# - Calling tick() for the first time, or after show() has returned, should always present the
#   window.
# - Calling close() from within the runloop started by show() should stop the runloop.
# - Calling close() between calls to tick() should make only the next call to tick() return false.

class TestWindow < Minitest::Test
  class ShyWindow < Gosu::Window
    def needs_redraw?
      false
    end
  end

  def test_reopen
    ShyWindow.new(200, 150).tick
  end
end
