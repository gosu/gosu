# Do not copy gosu.bundle into this template.
# This 'require' is only here so you can run your game from the
# terminal using `ruby Main.rb`, or your favorite text editor.
require 'rubygems'
require 'gosu'

class MyWindow < Gosu::Window
  def initialize
    super 640, 480, false
    
    self.caption = "Your Gosu game goes here"
    
    # ...
  end
  
  def draw
    # ...
  end
  
  def update
    # ...
  end
end

MyWindow.new.show
