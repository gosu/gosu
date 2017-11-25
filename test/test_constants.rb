# Encoding: UTF-8
require_relative "test_helper"

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
  end
end
