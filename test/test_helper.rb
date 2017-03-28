gem 'minitest'
require "minitest/autorun"
require "gosu" unless defined? Gosu

module TestHelper
  # TODO: Should be __dir__ after we drop Ruby 1.x support...
  def media_path(fname='')
    File.join(File.dirname(__FILE__), "media", fname)
  end
end

module Skipper
  def skip_on_appveyor
    skip if ENV['APPVEYOR']
  end

  def skip_on_travis
    skip if ENV['TRAVIS']
  end

  def skip_on_ci
    skip if ENV['APPVEYOR'] or ENV['TRAVIS']
  end
end

module InteractiveTests
  def interactive_cli(message)
    return false unless ENV["GOSU_TEST_INTERACTIVE"]
    
    STDOUT.puts message + 'Type (Y)es or (N)o  or (S)kip and ENTER'
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
    
    STDOUT.puts message + 'Press (Y)es or (N)o on your Keyboard'
    win = yield
    win.extend InteractiveWindow
    
    assert_output "Tester answered 'Yes'", // do
      win.show
    end
  end
end

module InteractiveWindow
  def button_down(id)
    if id == Gosu::KB_Y
      puts "Tester answered 'Yes'" 
      close!
    elsif id == Gosu::KB_N
      puts "Tester answered 'No'"
    end
  end
  
  def close
    puts "Tester canceled the #{self.class}-Test"
    close!
  end  
end
