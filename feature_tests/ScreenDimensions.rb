$LOAD_PATH << '../lib'
require 'gosu'

class Test < Gosu::Window
  def initialize 
    super(Gosu::screen_width * 5 / 10, Gosu::screen_height * 5 / 10, false)
  end

  def button_down(id)
    close if id == Gosu::KbEscape
  end
end

Test.new.show
