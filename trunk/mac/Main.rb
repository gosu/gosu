# Do not include gosu.bundle into this template.
# This is just here so you can run your game from the terminal, or
# your favorite text editor, using `ruby Main.rb`.

require 'gosu'

class MyWindow < Gosu::Window
  def initialize
    super 640, 480, false
    
    self.caption = "Your Gosu game goes here"
  end
end

MyWindow.new.show
