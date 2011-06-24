$LOAD_PATH << '../lib'
require 'gosu'

class Test < Gosu::Window
  def initialize
    super(300, 300, false)
    @font_names = %w(Arial
                     Courier\ New
                     Georgia
                     Times\ New\ Roman
                     Verdana
                     Trebuchet\ MS
                     Lucida\ Sans).map do |name|
      Gosu::Image.from_text(self, name, name, 20) rescue Gosu::Image.from_text(self, "Missing: #{name}", Gosu::default_font_name, 20)
    end
  end

  def draw
    @font_names.each_with_index do |image, index|
      image.draw 10, 10 + index * 30, 0
    end
  end
end

Test.new.show
