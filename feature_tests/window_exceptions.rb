# Require cutting-edge development Gosu for testing.
$LOAD_PATH << '../lib'
require 'gosu'

# This kind of turned into a stress-test for Song too.

class Test < Gosu::Window
  def initialize 
    super 400, 300, false
  end
  
  def update
    raise "Error in update" if button_down? Gosu::KbU
  end
  
  def draw
    raise "Error in draw" if button_down? Gosu::KbD
  end

  def button_down id
    raise "Error in button_down" if id == Gosu::KbB
  end
end

Test.new.show
