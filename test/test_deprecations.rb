# Encoding: UTF-8

require "minitest/autorun"
require "gosu" unless defined? Gosu

class TestDeprecations < Minitest::Test
  def test_gosu_module_constants
    # test only a few Button constants as they're automatically generated, so if these works, all other constants should work too
    assert_output("", /DEPRECATION WARNING: Gosu::Kb0 is deprecated; use KB_0 instead./) { Gosu::Kb0 }
    assert_output("", /DEPRECATION WARNING: Gosu::KbNumpadSubtract is deprecated; use KB_NUMPAD_MINUS instead./) { Gosu::KbNumpadSubtract }
    assert_output("", /DEPRECATION WARNING: Gosu::MsLeft is deprecated; use MS_LEFT instead./) { Gosu::MsLeft }
    assert_output("", /DEPRECATION WARNING: Gosu::GpDown is deprecated; use GP_DOWN instead./) { Gosu::GpDown }
    assert_output("", /DEPRECATION WARNING: Gosu::Gp0Left is deprecated; use GP_0_LEFT instead./) { Gosu::Gp0Left }
    assert_output("", /DEPRECATION WARNING: Gosu::Gp1Right is deprecated; use GP_1_RIGHT instead./) { Gosu::Gp1Right }
    assert_output("", /DEPRECATION WARNING: Gosu::Gp2Up is deprecated; use GP_2_UP instead./) { Gosu::Gp2Up }
    assert_output("", /DEPRECATION WARNING: Gosu::Gp3Button0 is deprecated; use GP_3_BUTTON_0 instead./) { Gosu::Gp3Button0 }
  end

  def test_only_warn_once_per_origin
    assert_output("", /DEPRECATION WARNING/) { single_origin }
    assert_silent { single_origin }
  end

  def single_origin
    Gosu::KbEscape
    with_gosu_window do |win|
      win.button_id_to_char(Gosu::KB_A)
    end
  end

  def test_window_no_longer_needed
    with_gosu_window do |win|
      assert_output("", /DEPRECATION WARNING: Passing a Window to Sample#initialize has been deprecated in Gosu 0.7.17./) do
        assert_raises(::ArgumentError) { Gosu::Sample.new(win) }
      end

      assert_output("", /DEPRECATION WARNING: Passing a Window to Song#initialize has been deprecated in Gosu 0.7.17./) do
        assert_raises(::ArgumentError) { Gosu::Song.new(win) }
      end

      assert_output("", /DEPRECATION WARNING: Passing a Window to Image#initialize has been deprecated in Gosu 0.9./) do
        assert_raises(::NoMethodError) { Gosu::Image.new(win) }
      end

      assert_output("", /DEPRECATION WARNING: Passing a Window to Image.from_text has been deprecated in Gosu 0.9./) do
        assert_raises(::TypeError) { Gosu::Image.from_text(win,2,3,4) }
        assert_raises(::TypeError) { Gosu::Image.from_text(win,2,3,4,5,6,7) }
      end
    end
  end

  def test_moved_window_to_gosu_module_methods
    # Class methods that have been turned into module methods.
    assert_output("", /DEPRECATION WARNING: Gosu::Window::button_id_to_char is deprecated; use Gosu.button_id_to_char instead./) { Gosu::Window.button_id_to_char(Gosu::KB_A) }
    assert_output("", /DEPRECATION WARNING: Gosu::Window::char_to_button_id is deprecated; use Gosu.char_to_button_id instead./) { Gosu::Window.char_to_button_id("A") }

    # Instance methods that have been turned into module methods.
    # test only a few methods as they're automatically generated, so if these works, all other should work as well

    with_gosu_window do |win|
      %w(draw_line translate char_to_button_id).each do |meth|
        assert_output("", %r{DEPRECATION WARNING: Gosu::Window##{meth} is deprecated; use Gosu.#{meth} instead.}) do
          assert_raises(::ArgumentError) { win.send(meth) }
        end
      end
    end
  end

  # Catch a linux specific error
  def with_gosu_window
    yield Gosu::Window.new(10,10)
  rescue RuntimeError => e
    raise e unless e.message =~ /Could not initialize SDL Video/
  end
end
