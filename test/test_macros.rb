# Encoding: UTF-8
require_relative "test_helper"

class TestMacros < Minitest::Test
  include TestHelper

  class PrimitivesMacroWindow < Gosu::Window

    def initialize(width, height)
      super(width*4,height*2)

      # Define the elements as lambdas so the code can be reused as for the macros and the "direct" call in Window.draw
      @line = lambda{|dx=0,dy=0|
        dx ||= 0
        Gosu.draw_line(width/2+dx, height*9/10+dy, Gosu::Color::GREEN,
                       width/10+dx, height/10+dy, Gosu::Color::GREEN)
      }
      @triangle = lambda{|dx=0,dy=0|
        dx ||= 0
        Gosu.draw_triangle(width/10+dx, height/10+dy, Gosu::Color::BLUE,
                           width*9/10+dx, height/2+dy, Gosu::Color::BLUE,
                           width/2+dx, height*9/10+dy, Gosu::Color::BLUE)
      }
      @quad = lambda{|dx=0,dy=0|
        dx ||= 0
        Gosu.draw_quad(width/10+dx, height/10+dy, Gosu::Color::YELLOW,
                       width*9/10+dx, height/2+dy, Gosu::Color::YELLOW,
                       width*9/10+dx, height*9/10+dy, Gosu::Color::YELLOW,
                       width/2+dx, height*9/10+dy, Gosu::Color::YELLOW)
      }
      @rect = lambda{|dx=0,dy=0|
        dx ||= 0
        Gosu.draw_rect(width/10+dx, height/10+dy, width*8/10, height*8/10, Gosu::Color::RED);
      }

      # Generate macros for all
      @line_rec = record(width, height, &@line)
      @triangle_rec = record(width, height, &@triangle)
      @quad_rec = record(width, height, &@quad)
      @rect_rec = record(width, height, &@rect)
    end

    def draw
      dx = width/4
      dy = height/2

      @line.call(0,0)
      @triangle.call(dx,0)
      @quad.call(dx*2,0)
      @rect.call(dx*3,0)

      @line_rec.draw(0,dy,0)
      @triangle_rec.draw(dx,dy,0)
      @quad_rec.draw(dx*2,dy,0)
      @rect_rec.draw(dx*3,dy,0)
    end
  end

  def test_primitives
    window = PrimitivesMacroWindow.new(200, 200)
    assert_screenshot_matches window, 'drawing-primitives.png'
  end
end
