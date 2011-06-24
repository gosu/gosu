$LOAD_PATH << '../lib'
require 'gosu'

class NeedsCursorTest < Gosu::Window
  def initialize
    super 800, 600, false
  end
  
  def needs_cursor?
    puts button_down?(Gosu::KbSpace)
    button_down? Gosu::KbSpace
  end
end

NeedsCursorTest.new.show
