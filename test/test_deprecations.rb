require_relative "test_helper"

class TestDeprecations < Minitest::Test
  DUMMY_WINDOW = Gosu::Window.new(100, 100)

  def test_only_warn_once_per_origin
    assert_output("", /DEPRECATION WARNING/) { single_origin }
    assert_silent { single_origin }
  end

  def single_origin
    DUMMY_WINDOW.set_mouse_position(1, 2)
  end

  def test_window_no_longer_needed
    assert_output("", /DEPRECATION WARNING: Passing a Window to Sample#initialize has been deprecated in Gosu 0.7.17./) do
      assert_raises(::ArgumentError) { Gosu::Sample.new(DUMMY_WINDOW) }
    end

    assert_output("", /DEPRECATION WARNING: Passing a Window to Song#initialize has been deprecated in Gosu 0.7.17./) do
      assert_raises(::ArgumentError) { Gosu::Song.new(DUMMY_WINDOW) }
    end

    assert_output("", /DEPRECATION WARNING: Passing a Window to Image#initialize has been deprecated in Gosu 0.9./) do
      assert_raises(::ArgumentError) { Gosu::Image.new(DUMMY_WINDOW) }
    end

    assert_output("", /DEPRECATION WARNING: Passing a Window to Image.from_text has been deprecated in Gosu 0.9./) do
      assert_raises(::TypeError) { Gosu::Image.from_text(DUMMY_WINDOW, 2, 3, 4) }
      assert_raises(::TypeError) { Gosu::Image.from_text(DUMMY_WINDOW, 2, 3, 4, 5, 6, 7) }
    end
  end

  def test_options_hash_instead_of_keyword_arguments
    assert_output("", /Passing options as a Hash to Window#initialize/) do
      window = Gosu::Window.new(100, 100, { fullscreen: false, update_interval: 50, resizable: true })
      refute window.fullscreen?
      assert window.resizable?
      assert_in_delta 50, window.update_interval
    end

    blob_image = Gosu::Image.from_blob(1, 1, "\xff\x00\x00\xff")
    assert_output("", /Passing options as a Hash to Image#initialize/) do
      image = Gosu::Image.new(File.join(__dir__, "test_image/from_blob.png"), { retro: true })
      assert_equal Gosu::Image.new(File.join(__dir__, "test_image/from_blob.png")).width, image.width
    end

    assert_output("", /Passing options as a Hash to Image.load_tiles/) do
      assert_equal 1, Gosu::Image.load_tiles(blob_image, 1, 1, { tileable: true }).size
    end

    assert_output("", /Passing options as a Hash to Image.from_text/) do
      assert_equal 100, Gosu::Image.from_text("Hi", 20, { width: 100, align: :right }).width
    end

    assert_output("", /Passing options as a Hash to Image.from_markup/) do
      assert_equal 100, Gosu::Image.from_markup("<b>Hi</b>", 20, { width: 100 }).width
    end

    assert_output("", /Passing options as a Hash to Font#initialize/) do
      assert_equal 20, Gosu::Font.new(20, { bold: true }).height
    end

    assert_output("", /Passing options as a Hash to Gosu.render/) do
      assert_equal 10, Gosu.render(10, 10, { retro: true }) {}.width
    end
  end

  def test_old_window_signatures
    assert_output("", /Passing a Window to Image#initialize/) do
      # Image.new(window, filename) is valid, tileable used to be optional.
      assert_raises(::RuntimeError) { Gosu::Image.new(DUMMY_WINDOW, "does_not_exist.png") }
    end

    assert_output("", /Passing a Window to Image.load_tiles/) do
      assert_raises(::RuntimeError) { Gosu::Image.load_tiles(DUMMY_WINDOW, "does_not_exist.png", 1, 1, false) }
    end
  end
end
