# Based on the C Demo3 demonstration distributed with Chipmunk.
# Also with some help from the ChipmunkIntegration.rb program.
#
# License: Same as for Gosu (MIT)
# Created on 21/10/2007, 00:05:19 by Robert Sheehan

require 'rubygems'
require 'gosu'
require 'chipmunk'
require 'RMagick'

# Convenience method for converting from radians to a Vec2 vector.
class Numeric 
   def radians_to_vec2
       CP::Vec2.new(Math::cos(self), Math::sin(self))
   end
end

# Layering of sprites
module ZOrder
   Background, Box = *0..1
end

SCREEN_WIDTH = 640
SCREEN_HEIGHT = 480
TICK = 1.0/60.0
NUM_POLYGONS = 80
NUM_SIDES = 4
EDGE_SIZE = 15

# Everything appears in the Gosu::Window.
class DemoWindow < Gosu::Window

   def initialize
       super(SCREEN_WIDTH, SCREEN_HEIGHT, false)
       self.caption = "A Chipmunk-RMagick-Gosu Demonstration"
       @space = CP::Space.new
       @space.iterations = 5
       @space.gravity = CP::Vec2.new(0, 100)
       # you can replace the background with any image with this line
       #  background = Magick::ImageList.new( "media/Space.png")
       fill = Magick::TextureFill.new(Magick::ImageList.new("granite:"))
       background = Magick::Image.new(SCREEN_WIDTH, SCREEN_HEIGHT, fill)
       setup_triangles(background)
       @background_image = Gosu::Image.new(self, background, true) # turn the image into a Gosu one
       @boxes = create_boxes(NUM_POLYGONS)
   end

   # Create all of the static triangles.
   # Adds them to the space and the background image.
   def setup_triangles(background)
       gc = Magick::Draw.new
       gc.stroke_width(2)
       gc.stroke('red')
       gc.fill('blue')
       # all the triangles are part of the same body
       body = CP::Body.new(Float::MAX, Float::MAX)
       base = 15
       height = 10
       shape_vertices =  [CP::Vec2.new(-base, base), CP::Vec2.new(base, base), CP::Vec2.new(0, -height)]
       # make shapes and images
       9.times do |i|
           6.times do |j|
               stagger = (j % 2) * 40
               x = i * 80 + stagger
               y =  j * 70 + 80
               shape = CP::Shape::Poly.new(body, shape_vertices, CP::Vec2.new(x, y))
               shape.e = 1
               shape.u = 1
               @space.add_static_shape(shape)
               gc.polygon(x - base + 1, y + base - 1, x + base - 1, y + base - 1,  x, y - height + 1)
           end
       end
       # do the drawing
       gc.draw(background)
   end

   # Produces the vertices of a regular polygon.
   def polygon_vertices(sides, size)
       vertices = []
       sides.times do |i|
           angle = -2 * Math::PI * i / sides
           vertices << angle.radians_to_vec2() * size
       end
       return vertices
   end

   # Produces the image of a polygon.
   def polygon_image(vertices)
       box_image = Magick::Image.new(EDGE_SIZE  * 2, EDGE_SIZE * 2) { self.background_color = 'transparent' }
       gc = Magick::Draw.new
       gc.stroke('red')
       gc.fill('plum')
       draw_vertices = vertices.map { |v| [v.x + EDGE_SIZE, v.y + EDGE_SIZE] }.flatten
       gc.polygon(*draw_vertices)
       gc.draw(box_image)
       return Gosu::Image.new(self, box_image, false)
   end

   # Produces the polygon objects and adds them to the space.
   def create_boxes(num)
       box_vertices = polygon_vertices(NUM_SIDES, EDGE_SIZE)
       box_image = polygon_image(box_vertices)
       boxes =  []
       num.times do
           body = CP::Body.new(1, CP::moment_for_poly(1.0, box_vertices, CP::Vec2.new(0, 0))) # mass, moment of inertia
           body.p = CP::Vec2.new(rand(SCREEN_WIDTH), rand(40) - 50)
           shape = CP::Shape::Poly.new(body, box_vertices, CP::Vec2.new(0, 0))
           shape.e = 0.0
           shape.u = 0.4
           boxes << Box.new(box_image, body)
           @space.add_body(body)
           @space.add_shape(shape)      
       end
       return boxes
   end

   # All the simulation is done here.
   def update
       @space.step(TICK)
       @boxes.each { |box| box.check_off_screen }
   end

   # All the updating of the screen is done here.
   def draw
       @background_image.draw(0, 0, ZOrder::Background)
       @boxes.each { |box| box.draw }
   end

end

# The falling boxes class.
# Nothing more than a body and an image.
class Box

   def initialize(image, body)
       @image = image
       @body = body
   end

   # If it goes offscreen we put it back to the top.
   def check_off_screen
       pos = @body.p
       if pos.y > SCREEN_HEIGHT + EDGE_SIZE or pos.x > SCREEN_WIDTH + EDGE_SIZE or pos.x < -EDGE_SIZE
           @body.p = CP::Vec2.new(rand * SCREEN_WIDTH, 0)
       end
   end

   def draw
       @image.draw_rot(@body.p.x, @body.p.y, ZOrder::Box, @body.a.radians_to_gosu)
   end
end

window = DemoWindow.new
window.show
