# Encoding: UTF-8

require "minitest/autorun"
require "gosu" unless defined? Gosu

class TestWindow < Minitest::Test
  class CallbackTestWindow < Gosu::Window
    attr_accessor :stop
    def initialize
      super(10,10)
      @stop = false
    end

    def close?
      puts @stop ? 'Ok lets go.' : 'You shall not pass!'
      @stop
    end
  end

  def test_close_callback
    assert_output /You shall not pass!\nOk lets go./, '' do
      win = CallbackTestWindow.new
      Thread.new(win) do
        sleep 0.5
        win.close
        sleep 0.5
        win.stop = true
        win.close
      end
      win.show
    end
  end
end
