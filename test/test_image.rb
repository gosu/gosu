# Encoding: UTF-8
require_relative "test_helper"

class TestImageIO < Minitest::Test
  include TestHelper

  #----------- Creating and loading images
  # def test_from_text -> see test_text

  def test_load_tiles
    # Basic usage
    tiles = Gosu::Image.load_tiles(media_path('rotation.png'), 50, 50)
    assert tiles.is_a?(Array)
    assert_equal 4, tiles.size
    tiles.each do |t|
      assert t.is_a?(Gosu::Image)
      assert_equal 50, t.width
      assert_equal 50, t.height
    end

    win = ScreenshotWindow.draw(100, 100) do
      tiles.each_with_index do |t,idx|
        x = (idx % 2) * 50
        y = (idx / 2) * 50
        t.draw(x, y, 0)
      end
    end
    assert_screenshot_matches win, 'rotation.png'

    # with retro: true
    tiles = Gosu::Image.load_tiles(media_path('rotation.png'), 50, 50)
    retro_tiles = Gosu::Image.load_tiles(media_path('rotation.png'), 50, 50, retro: true)
    win = ScreenshotWindow.draw(250, 100) do
      retro_tiles[0].draw(-80, 0, 0, scale: 4.0)
      tiles[0].draw(50, 0, 0, scale_x: 4.0, scale_y: 4.0, mode: :add)

      retro_tiles[1].draw(0, 0, 0, scale: 0.5)
      tiles[1].draw(50, 0, 0, scale_x: 0.5, scale_y: 0.5)
    end
    assert_screenshot_matches win, 'retro.png'

    # with tileable: true
    tileable_tiles = Gosu::Image.load_tiles(media_path('rotation.png'), 50, 50, tileable: true)
    win = ScreenshotWindow.draw(250, 100) do
      tileable_tiles[0].draw(-80, 0, 0, scale_x: 4.0, scale_y: 4.0)
      tiles[0].draw(50, 0, 0, scale_x: 4.0, scale_y: 4.0, mode: :add)

      tileable_tiles[1].draw(0, 0, 0, scale_x: 0.5, scale_y: 0.5)
      tiles[1].draw(50, 0, 0, scale_x: 0.5, scale_y: 0.5)
    end
    assert_screenshot_matches win, 'tileable.png', 0.98
  end

  def test_new_with_rect
    Dir.chdir media_path do
      Dir.mktmpdir do |tmpdir|
        output_filename = File.join(tmpdir, 'new_with_rect.bmp')

        image = Gosu::Image.new(media_path('inserted.png'), rect: [0, 0, 25, 25])
        image.save(output_filename)
        assert_equal_files 'triangle-25.bmp', output_filename
      end
    end
  end

  def test_subimage
    win = ScreenshotWindow.draw(50, 50) do |w, h|
      c = 0xff_ffffff
      @image ||= Gosu::Image.new(media_path('triangle-50.png'))
      @subimage ||= @image.subimage(15, 15, 20, 20)
      @image.draw(0, 0, 0)
      @subimage.draw(25, 25, 0, angle: 180)
    end
    assert_screenshot_matches win, 'trintrang.png'
  end

  def test_from_blob
    img = Gosu::Image.new(media_path('triangle-25.bmp'))
    assert_equal img.to_blob, Gosu::Image.from_blob(img.to_blob, 25, 25).to_blob

    # TODO: "Fix" this test. so far it looks like the resulting image is different every time I rerun the test Oo?
    # Maybe use a real 128bit image as base so the differences are easier to spot - at the moment it 95% transparent
    # with a few dots here and there.
    Gosu::Image.from_blob(img.to_blob*4, 25, 25)

    assert_raises(::ArgumentError) { Gosu::Image.from_blob(img.to_blob, 26, 25) }
    assert_raises(::ArgumentError) { Gosu::Image.from_blob(img.to_blob*2, 25, 25) }
  end

  #----------- Drawing an image

  def test_draw_with_opts
    win = ScreenshotWindow.draw(50, 50) do
      @image ||= Gosu::Image.new(media_path('triangle-50.png'))
      @image.draw(0, 0, 0, scale_x: 1, scale_y: 1, color: 0xff_ffffff, mode: :default)
    end
    assert_screenshot_matches win, 'triangle-50.png'


    image = Gosu::Image.new(media_path('triangle-50.png'))
    assert_raises(::ArgumentError) { image.draw(0, 0, 0, unknown_param: 1) }
  end

  def test_draw_args_for_compat
    win = ScreenshotWindow.draw(50, 50) do
      @image ||= Gosu::Image.new(media_path('triangle-50.png'))
      @image.draw(0, 0, 0, 1, 1, 0xff_ffffff, :default)
    end
    assert_output("", /DEPRECATION WARNING:/) do
      assert_screenshot_matches win, 'triangle-50.png'
    end
  end

  def test_draw_with_angle
    win = ScreenshotWindow.draw(100, 100) do
      @image ||= Gosu::Image.new(media_path('triangle-50.png'))
      @image.draw(25, 25, 0, angle: 90)
      @image.draw(50, 25, 0, angle: 180, center_x: 1.0)
      @image.draw(50, 50, 0, angle: 215, center_y: 1.0)
      @image.draw(50, 50, 0, center: -1, scale: 0.5, color: 0xff_ffffff, mode: :default)
    end
    assert_screenshot_matches win, 'rotation.png'
  end

  # FIXME
  def test_draw_as_quad
    skip

    win = ScreenshotWindow.draw(100, 100) do |w, h|
      c = 0xff_ffffff
      @image ||= Gosu::Image.new(media_path('triangle-50.png'))
      @image.draw_as_quad(0, 0, c, w, 0, c, w, h, c, w-35, 15, c, 0, :default)

      # BUG: Drawing the quad from bottom to top is buggy (last Vertex is always wrong)
      # should     1--2
      # be:       /    \
      #          3------4
      @image.draw_as_quad(15, h/2, c, w-15, h/2, c, 0, h, c, w, h, c, 0, :default)
    end
    assert_screenshot_matches win, 'iso.png'
  end


  def test_blend_modes
    win = ScreenshotWindow.draw(100,100) do
      image = Gosu::Image.new(media_path('triangle-50.png'))

      image.draw(0, 0, 0)
      image.draw(0, 0, 0, mode: :default, angle: 180, center: :bottom_right)

      image.draw(50, 0, 0)
      image.draw(50, 0, 0, mode: :add, angle: 180, center: :bottom_right)

      image.draw(0, 50, 0)
      image.draw(0, 50, 0, mode: :multiply, angle: 180, center: :bottom_right)
    end
    assert_screenshot_matches win, 'blendmodes.png'
  end

  #----------- other instance methods

  def test_gl_tex_info
    gti = Gosu::Image.new(media_path('triangle-25.bmp')).gl_tex_info
    assert gti.is_a?(Gosu::GLTexInfo)
  end

  def test_insert
    Dir.chdir media_path do
      Dir.mktmpdir do |tmpdir|
        output_filename = File.join(tmpdir, 'is_insert.png')

        # Insert from filename
        image = Gosu::Image.new(media_path('triangle-50.png'))
        image.insert(media_path('triangle-25.bmp'), 0, 0)
        image.save(output_filename)
        assert_equal_files 'inserted.png', output_filename

        # Insert from Gosu::Image-object
        image = Gosu::Image.new(media_path('triangle-50.png'))
        image2 = Gosu::Image.new(media_path('triangle-25.bmp'))
        image.insert(image2, 0, 0)
        image.save(output_filename)
        assert_equal_files 'inserted.png', output_filename
      end
    end
  end

  # All images in each group (key) are saved and then compared to the expected image (value).
  # The alpha group uses bmp24 for comparison because all 'empty' pixels will reliably be 00000000.
  # The no-alpha group uses png32 for comparison because it is lossless.
  GROUPS = { "alpha" => "alpha-bmp24.bmp", "no-alpha" => "no-alpha-png32.png" }

  def test_save_roundtrip
    Dir.chdir media_path do
      Dir.mktmpdir do |tmpdir|
        GROUPS.each do |group, expected|
          dimensions = nil

          Dir.glob("#{group}-*.*") do |filename|
            image = Gosu::Image.new(filename)

            if dimensions.nil?
              dimensions = [image.width, image.height]
            else
              assert_equal dimensions, [image.width, image.height]
            end

            output_filename = File.join(tmpdir, expected)
            image.save output_filename
            assert_equal_files expected, output_filename
          end
        end
      end
    end
  end

  def test_to_blob
    image = Gosu::Image.new(media_path('triangle-25.bmp'))
    assert image.to_blob.is_a? String
    assert_equal image.width * image.height * 4, image.to_blob.size
  end
end
