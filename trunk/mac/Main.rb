# Do not copy gosu.bundle into this template.
# This 'require' is only here so you can run your game from the
# terminal using `ruby Main.rb`, or your favorite text editor.
require 'gosu'

class MyWindow < Gosu::Window
  def initialize
    super 640, 480, false
    
    self.caption = "Your Gosu game goes here"
  end
end

MyWindow.new.show
