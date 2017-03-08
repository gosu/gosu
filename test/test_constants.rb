# Encoding: UTF-8

require "minitest/autorun"
require "gosu" unless defined? Gosu

class TestConstants < Minitest::Test
  def test_version
    assert_match(/\A#{Gosu::MAJOR_VERSION}.#{Gosu::MINOR_VERSION}.#{Gosu::POINT_VERSION}/,
      Gosu::VERSION)
  end
 
  def test_licenses
    assert_match(/Gosu/, Gosu::LICENSES)
    assert_match(/SDL/, Gosu::LICENSES)
    unless RUBY_PLATFORM =~ /darwin/
      assert_match(/libsndfile/, Gosu::LICENSES)
      assert_match(/OpenAL/, Gosu::LICENSES) if RUBY_PLATFORM =~ /win/
    end

    # Backward compatibility
    assert_output "", /DEPRECATION WARNING: Gosu::GOSU_COPYRIGHT_NOTICE is deprecated; use LICENSES instead./ do
      assert_equal Gosu::LICENSES, Gosu::GOSU_COPYRIGHT_NOTICE
    end
  end
end
