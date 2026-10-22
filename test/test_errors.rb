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
end
