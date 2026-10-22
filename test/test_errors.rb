require_relative "test_helper"

class TestErrors < Minitest::Test
  # Every FFI function must reset the last error, so that an error does not get reported twice.
  def test_errors_are_not_reported_twice
    assert_raises(RuntimeError) { Gosu::Image.new("does_not_exist.png") }

    # ...other functions that check the last error...
    color = Gosu::Color.from_ahsv(0x80, 120, 0.5, 0.5)
    color.gl

    assert_raises(RuntimeError) { Gosu::Image.new("does_not_exist.png") }
    assert_equal 1, Gosu::Color.rgb(1, 2, 3).red
  end

  # Errors in Gosu callbacks that Ruby handles itself must not make the outer call fail.
  def test_handled_errors_in_callbacks_are_not_reported_again
    Gosu.render(1, 1) { Gosu::Image.new("does_not_exist.png") rescue nil }

    window_class = Class.new(Gosu::Window) do
      define_method(:update) { Gosu::Sample.new("does_not_exist.wav") rescue nil }
      define_method(:draw) { Gosu::Song.new("does_not_exist.ogg") rescue nil }
    end
    window = window_class.new(10, 10)
    2.times { window.tick }
  end

  class FailingTextInput < Gosu::TextInput
    def filter(text)
      raise "filter failed"
    end
  end

  # TextInput#filter is called from C++. Its exceptions are re-raised by Gosu::Window.
  def test_exceptions_in_text_input_filter
    window = Gosu::Window.new(10, 10)
    window.text_input = FailingTextInput.new
    # This is what happens when C++ calls the filter during Window#tick.
    window.text_input.send(:protected_filter, "abc")

    error = assert_raises(RuntimeError) { window.tick }
    assert_equal "filter failed", error.message
  end

  # After raising an exception from a callback, the window should not keep raising it.
  def test_window_exceptions_are_only_raised_once
    window_class = Class.new(Gosu::Window) do
      define_method(:update) do
        next if @raised

        @raised = true
        raise "update failed"
      end
    end
    window = window_class.new(10, 10)

    error = assert_raises(RuntimeError) { window.tick }
    assert_equal "update failed", error.message
    window.tick
  end

  # Gosu.gl(z) blocks are called from C++ after Window#draw.
  # Their exceptions are re-raised by Gosu::Window.
  def test_exceptions_in_gl_blocks
    window_class = Class.new(Gosu::Window) do
      attr_reader :frames

      define_method(:update) { @frames = (@frames || 0) + 1; close! if @frames == 10 }
      define_method(:draw) { Gosu.gl(0) { raise "gl failed" } if @frames == 2 }
    end

    window = window_class.new(10, 10)
    error = assert_raises(RuntimeError) { window.show }
    assert_equal "gl failed", error.message
    assert_equal 2, window.frames

    window = window_class.new(10, 10)
    window.tick
    error = assert_raises(RuntimeError) { window.tick }
    assert_equal "gl failed", error.message
  end

  # Gosu.gl(z) blocks must not be garbage collected before C++ calls them.
  def test_gl_blocks_survive_gc
    calls = 0
    window_class = Class.new(Gosu::Window) do
      define_method(:draw) do
        Gosu.gl(0) { GC.start; calls += 1 }
        10.times { Gosu.gl(1) { calls += 1 } }
      end
    end

    window = window_class.new(10, 10)
    3.times { window.tick }
    assert_equal 33, calls
  end

  # Like in Gosu 1.4.6 (which used rb_yield), calling these methods without a block raises LocalJumpError.
  def test_missing_blocks
    assert_raises(LocalJumpError) { Gosu.render(1, 1) }
    assert_raises(LocalJumpError) { Gosu.record(1, 1) }
    Gosu.render(1, 1) do
      assert_raises(LocalJumpError) { Gosu.gl }
      assert_raises(LocalJumpError) { Gosu.gl(0) }
      assert_raises(LocalJumpError) { Gosu.transform([1] * 16) }
      assert_raises(LocalJumpError) { Gosu.translate(1, 1) }
      assert_raises(LocalJumpError) { Gosu.rotate(45) }
      assert_raises(LocalJumpError) { Gosu.scale(2) }
      assert_raises(LocalJumpError) { Gosu.clip_to(0, 0, 1, 1) }
    end
  end
end
