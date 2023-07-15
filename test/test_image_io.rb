require_relative "test_helper"

class TestImageIO < Minitest::Test
  include TestHelper
  
  # All images in each group (key) are saved and then compared to the expected image (value).
  # The alpha group uses bmp24 for comparison because all 'empty' pixels will be turned into fuchsia.
  # The no-alpha group uses png32 for comparison because it is lossless.
  GROUPS = { "alpha" => "alpha-bmp24.bmp", "no-alpha" => "no-alpha-png32.png" }

  def test_image_roundtrip
    Dir.chdir File.join(File.dirname(__FILE__), "test_image_io") do
      Dir.mktmpdir do |tmpdir|
        GROUPS.each do |group, expected|
          dimensions = nil

          Dir.glob("#{group}-*.*") do |filename|
            image = Gosu::Image.new(filename)

            dimensions ||= [image.width, image.height]
            assert_equal dimensions, [image.width, image.height],
                "#{filename} had unexpected dimensions after loading"

            output_filename = File.join(tmpdir, expected)
            image.save output_filename
            assert_equal [File.binread(expected)].pack("m"), [File.binread(output_filename)].pack("m"),
                "#{filename} was different from reference image (#{expected}) when written to file"
          end
        end
      end
    end
  end
end
