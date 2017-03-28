gem 'minitest'
require "minitest/autorun"
require "gosu" unless defined? Gosu

module InteractiveTests
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