require 'gosu'

class MyWindow < Gosu::Window
  def initialize(width, height, fullscreen, tick_len)
    super
    
    # your initialization code here
  end
  
  def update
    # your game logic here
  end
  
  def draw
    # your game visuals here
  end
  
  def button_down(id)
    # your button handling code here
  end
end

w = MyWindow.new(640, 480, false, 20)
w.show