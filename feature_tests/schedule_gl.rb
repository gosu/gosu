$LOAD_PATH << '../lib'
require 'gosu'
require 'rubygems'
require 'opengl'

class ScheduleGLTest < Gosu::Window
  def initialize
    super 800, 600, false
    image = Gosu::Image.new self, "media/Cursor.png", false
    @images = Array.new(1000) do
      x, y, z = rand(800), rand(600), rand * 10 - 1
      lambda { image.draw x, y, z }
    end
    @gl_z = 0
  end
  
  def draw
    @images.each &:call
    gl(@gl_z) { Gosu::undocumented_glclear }
  end
  
  def needs_cursor?; true; end
  
  def button_down id
    if id == Gosu::MsWheelDown then
      @gl_z += 1
    elsif id == Gosu::MsWheelUp then
      @gl_z -= 1
    end
  end
end

ScheduleGLTest.new.show
