$LOAD_PATH << '../lib'
require 'gosu'

class Test < Gosu::Window
  def initialize 
    super(300, 200, false, 20)
  end
    
  def button_down(id)        
    self.caption = "down" if id == Gosu::MsWheelDown
    self.caption = "up" if id == Gosu::MsWheelUp
  end
end

Test.new.show
