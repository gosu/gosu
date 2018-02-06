gem "minitest"
require "minitest/autorun"

require "gosu" unless defined? Gosu

class Gosu::Image
  # Gosu does not implement this method by default because it is very inefficient.
  # However, it is useful for testing, and makes it easy to use assert_equal on images.
  def ==(other)
    if other.is_a? Gosu::Image
      (to_blob rescue object_id) == (other.to_blob rescue other.object_id)
    else
      false
    end
  end
end

module TestHelper
  # TODO: Should be __dir__ after we drop Ruby 1.x support...
  def media_path(fname = "")
    File.join(File.dirname(__FILE__), "media", fname)
  end

  def skip_on_appveyor
    skip if ENV["APPVEYOR"]
  end

  def skip_on_travis
    skip if ENV["TRAVIS"]
  end

  def skip_on_ci
    skip if ENV["APPVEYOR"] or ENV["TRAVIS"]
  end
end

module InteractiveTests
  def interactive_cli(message)
    return false unless ENV["GOSU_TEST_INTERACTIVE"]
    
    STDOUT.puts message + "Type (Y)es or (N)o or (S)kip and ENTER"
    yield if block_given?
    
    user_input = STDIN.gets
    if user_input =~ /[sS]/
      skip
    else
      assert user_input =~ /[yY]/, "User answered 'No' to '#{message}'"
    end
  end
  
  def interactive_gui(message)
    return false unless ENV["GOSU_TEST_INTERACTIVE"]
    
    STDOUT.puts message + "Press (Y)es or (N)o on your keyboard"
    win = yield
    win.extend InteractiveWindow
    
    assert_output "User answered 'Yes'\n", // do
      win.show
    end
  end
end

module InteractiveWindow
  def button_down(id)
    case Gosu.button_id_to_char(id)
    when "y"
      puts "User answered 'Yes'" 
      close!
    when "n"
      puts "User answered 'No'"
    end
  end
  
  def close
    puts "User canceled the test #{self.class}"
    close!
  end  
end
