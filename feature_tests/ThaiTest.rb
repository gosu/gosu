# Encoding: UTF-8

require 'rubygems'
require 'gosu'

class W < Gosu::Window
  def initialize
    super 640, 480, false
    self.caption = "มฃฃฌาฬิ์ฬฬท็"
    @f = Gosu::Font.new(self, "KodchiangUPC", 20)
    @i = Gosu::Image.from_text(self, "มฃฃฌาฬิ์ฬฬท็", "KodchiangUPC", 20, 0, 800, :center)
  end
  
  def draw
    @f.draw "ม", 0, 0, 0
    @i.draw 0, 300, 0
  end
end

W.new.show
