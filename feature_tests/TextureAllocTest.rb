$LOAD_PATH << '../lib'
require 'gosu'
require 'rubygems'
require 'spec'

class Test < Gosu::Window
  def initialize
    super(800, 600, false)
    self.caption = "Texture Alloc Test"
  end
end

window = Test.new
describe "Gosu Texture Allocation" do
  it "should allocate square power-of-two textures as a full texture" do
    img = Gosu::Image.new(window, "media/SquareTexture.png", true)
    img.gl_tex_info.left.should == 0
    img.gl_tex_info.right.should == 1
    img.gl_tex_info.top.should == 0
    img.gl_tex_info.bottom.should == 1
  end
  
  it "should not do the same for textures without hard borders because it needs room for blurring" do
    img = Gosu::Image.new(window, "media/SquareTexture.png", false)
    img.gl_tex_info.left.should_not == 0
    img.gl_tex_info.right.should_not == 1
    img.gl_tex_info.top.should_not == 0
    img.gl_tex_info.bottom.should_not == 1
  end

  it "should not allocate textures with weird sizes on full textures" do
    img = Gosu::Image.new(window, "media/Cursor.png", false)
    [img.gl_tex_info.left, img.gl_tex_info.right].should_not == [0, 1]
    [img.gl_tex_info.top, img.gl_tex_info.bottom].should_not == [0, 1]
  end
end

