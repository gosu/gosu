require_relative "test_helper"

class TestImage < Minitest::Test
  include TestHelper
  
  private def circle_image(radius)
    # Opaque blue as an RGBA string (0x0000ffff).
    inside = "\x00\x00\xff\xff".force_encoding(Encoding::BINARY)
    # Transparent blue as an RGBA string (0x0000ff00).
    outside = 0x00.chr + 0xff.chr + 0xff.chr + 0x00.chr

    # This creates a binary string that contains a circle, thanks to the
    # Pythagorean theorem.
    rgba = (-radius..+radius).map do |y|
      # 'x' is the distance from the center to the edges edge of the circle.
      x = Math.sqrt(radius ** 2 - y ** 2).round
      outside_circle = outside * (radius - x)
      inside_circle = inside * (x * 2 + 1)
      outside_circle + inside_circle + outside_circle
    end.join
    
    # The image size is (radius * 2 + 1), not (radius * 2).
    size = radius * 2 + 1

    Gosu::Image.from_blob(size, size, rgba)
  end
  
  def test_image_from_blob
    assert_image_matches "test_image/from_blob", circle_image(70), 1.00
  end

  LooksLikeRMagickImage = Struct.new(:columns, :rows, :to_blob)

  def test_image_roundtrips
    circle = circle_image(70)
    
    padded_circle = Gosu::Image.from_blob(circle.width + 4, circle.height + 2)
    padded_circle.insert circle, 2, 1
    
    # Convert to blob and back.
    fake_rmagick_image = LooksLikeRMagickImage.new(padded_circle.width, padded_circle.height, padded_circle.to_blob)
    circle2 = Gosu::Image.new(fake_rmagick_image, rect: [2, 1, circle.width, circle.height])
    assert circle.similar?(circle2, 1.00)

    # Convert to file and back.
    temporary_filename = "#{Dir.tmpdir}/gosu_test_image_roundtrip.png"
    padded_circle.save(temporary_filename)
    circle3 = Gosu::Image.new(temporary_filename, rect: [2, 1, circle.width, circle.height])
    assert circle.similar?(circle3, 1.00)
    File.delete temporary_filename
  end
  
  private def rect_image(w, h)
    red = 0xff.chr + 0x00.chr + 0x00.chr + 0xff.chr
    white = 0xff.chr * 4
    rgba = red * w +
           (red + white * (w - 2) + red) * (h - 2) +
           red * w
    Gosu::Image.from_blob(w, h, rgba)
  end
  
  # This uses large images so it implicitly tests the whole stack:
  # - Ruby wrapper for Gosu::Drawable::insert
  # - Gosu::TiledDrawable::insert
  # - Gosu::TexChunk::insert
  # - Gosu::Bitmap::insert
  def test_image_insert
    canvas = Gosu::Image.from_blob(3000, 2000)
    stamp = rect_image(1337, 1337)
    8.times do |i|
      canvas.insert stamp, i * 300, i * 123
    end
    assert_image_matches "test_image/insert", canvas, 1.00
  end

  def test_subimage
    filename = File.join(File.dirname(__FILE__), "test_image_io/no-alpha-jpg.jpg")
    image = Gosu::Image.new(filename)
    
    # Passing an out-of-bound rectangle is not allowed.
    assert_raises(Exception) { image.subimage(0, 0, 100, 1000) }

    # Passing a rectangle that is entirely within the image is allowed.
    subimage = image.subimage(1, 2, 99, 100)
    assert_equal 99, subimage.width
    assert_equal 100, subimage.height

    image_from_rect = Gosu::Image.new(filename, rect: [1, 2, 99, 100])
    assert image_from_rect.similar?(subimage, 1.00)
  end
end
