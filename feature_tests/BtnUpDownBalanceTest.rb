$LOAD_PATH << '../lib'
require 'gosu'

class Test < Gosu::Window
  def initialize
    super(300, 200, false, 20)
    @downs = @ups = 0
  end
  
  def button_down id
    @downs += 1
    puts "Button down: #{id}"
  end
  
  def button_up id
    @ups += 1
    puts "Button up: #{id}"
  end
  
  def update
    self.caption = "#{@downs} downs, #{@ups} ups"
  end
end

Test.new.show
