class Window
  def _objcObject
    @ref
  end
  
  def initialize(width, height, fullscreen, update_interval = 16.666666)
    @ref = ::ObjGosuWindow.alloc.initWithDelegate self, width: width, height: height,
      inFullscreen: fullscreen, updateInterval: update_interval
  end
  
  def caption; @ref.caption; end
  def caption=(s); @ref.setCaption s; end
    
  def show; @ref.show; end
  def close; @ref.close; end
  
  def mouse_x; @ref.mouseX; end
  def mouse_y; @ref.mouseY; end
  
  def buttonDown id
    button_down id
  end
  
  def buttonUp id
    button_up id
  end
  
  def button_up id; end
  def button_down id; end
  def draw; end
  def update; end
  def needs_redraw?; false; end
  
  def button_down? id
    @ref.isButtonDown id
  end
  
#     attr_accessor :caption
#     attr_accessor :mouse_x
#     attr_accessor :mouse_y
#     attr_accessor :text_input
#     attr_reader :width, :height
#     attr_reader :update_interval
#     
#     # update_interval:: Interval in milliseconds between two calls
#     # to the update member function. The default means the game will run
#     # at 60 FPS, which is ideal on standard 60 Hz TFT screens.
#     
#     # Enters a modal loop where the Window is visible on screen and receives calls to draw, update etc.
#     def show; end
# 
#     # Tells the window to end the current show loop as soon as possible.
#     def close; end 
#     
#     # Called every update_interval milliseconds while the window is being
#     # shown. Your application's main game logic goes here.
#     def update; end
#     
#     # Called after every update and when the OS wants the window to
#     # repaint itself. Your application's rendering code goes here.
#     def draw; end
#     
#     # Can be overriden to give the game a chance to say no to being redrawn.
#     # This is not a definitive answer. The operating system can still cause
#     # redraws for one reason or another.
#     #
#     # By default, the window is redrawn all the time (i.e. Window#needs_redraw?
#     # always returns true.)
#     def needs_redraw?; end
#     
#     # DEPRECATED.
#     def set_mouse_position(x, y); end
#     
#     # Called before update when the user pressed a button while the
#     # window had the focus.
#     def button_down(id); end
#     # Same as buttonDown. Called then the user released a button.
#     def button_up(id); end
#     
#     # Draws a line from one point to another (last pixel exclusive).
#     def draw_line(x1, y1, c1, x2, y2, c2, z=0, mode=:default); end
#       
#     def draw_triangle(x1, y1, c1, x2, y2, c2, x3, y3, c3, z=0, mode=:default); end
#     
#     # Draws a rectangle (two triangles) with given corners and corresponding
#     # colors.
#     # The points can be in clockwise order, or in a Z shape.
#     def draw_quad(x1, y1, c1, x2, y2, c2, x3, y3, c3, x4, y4, c4, z=0, mode=:default); end
#     
#     # Returns true if a button is currently pressed. Updated every tick.
#     def button_down?(id); end
#     
#     # DEPRECATED: Returns the character a button usually produces, or nil. Please use TextInput instead.
#     def self.button_id_to_char(id); end
#     
#     # DEPRECATED: Returns the button that has to be pressed to produce the
#     # given character, or nil. Please use TextInput instead.
#     def self.char_to_button_id(char); end
#     
#     # See examples/OpenGLIntegration.rb.
#     def gl(&custom_gl_code); end
#     
#     # Limits the drawing area to a given rectangle while evaluating the code inside of the block.
#     def clip_to(x, y, w, h, &drawing_code); end
#   end
#   
#   # Contains information about the underlying OpenGL texture and the u/v space used for image data.
#   #
#   # Can be retrieved from some images to use them in OpenGL operations. nil will be returned instead by images that are too large for a single texture.)
#   #
#   # See examples/OpenGLIntegration.rb.
#   class GLTexInfo
#     attr_reader :tex_name, :left, :right, :top, :bottom
#   end
#   
#   # Returns a random double between min (inclusive) and max (exclusive).
#   def random(min, max); end
#   
#   # Returns the horizontal distance between the origin and the point to which you would get if you moved radius pixels in the direction specified by angle.
#   def offset_x(angle, dist); end 
# 
#   # Returns the vertical distance between the origin and the point to which you would get if you moved radius pixels in the direction specified by angle.
#   def offset_y(angle, dist); end
# 
#   # Returns the angle from point 1 to point 2 in degrees, where 0.0 means upwards. Returns 0 if both points are equal.
#   def angle(x1, y1, x2, y2); end
# 
#   # Returns the smallest angle that can be added to angle1 to get to angle2 (can be negative if counter-clockwise movement is shorter).
#   def angle_diff(angle1, angle2); end
# 
#   # Returns the distance between two points.
#   def distance(x1, y1, x2, y2); end
# 
#   # Incrementing, possibly wrapping millisecond timer.
#   def milliseconds(); end
# 
#   # Returns the name of a neutral font that is available on the current
#   # platform.
#   def default_font_name(); end
# 
#   # Returns the width, in pixels, of the user's primary screen.
#   def screen_width(); end
# 
#   # Returns the height, in pixels, of the user's primary screen.
#   def screen_height(); end
end