# Version string of the form "0.1.2" or "0.1.2.3".
GOSU_VERSION = :a_string
# First component of the version.
GOSU_MAJOR_VERSION = :a_fixnum
# Second component of the version.
GOSU_MINOR_VERSION = :a_fixnum
# Third component of the version.
GOSU_POINT_VERSION = :a_fixnum
# A long block of legal copy that your game is obliged to display somewhere.
GOSU_COPYRIGHT_NOTICE = :a_string

module Gosu
  Kb0 = :implementation_defined
  Kb1 = :implementation_defined
  Kb2 = :implementation_defined
  Kb3 = :implementation_defined
  Kb4 = :implementation_defined
  Kb5 = :implementation_defined
  Kb6 = :implementation_defined
  Kb7 = :implementation_defined
  Kb8 = :implementation_defined
  Kb9 = :implementation_defined
  KbA = :implementation_defined
  KbB = :implementation_defined
  KbC = :implementation_defined
  KbD = :implementation_defined
  KbE = :implementation_defined
  KbF = :implementation_defined
  KbG = :implementation_defined
  KbH = :implementation_defined
  KbI = :implementation_defined
  KbJ = :implementation_defined
  KbK = :implementation_defined
  KbL = :implementation_defined
  KbM = :implementation_defined
  KbN = :implementation_defined
  KbO = :implementation_defined
  KbP = :implementation_defined
  KbQ = :implementation_defined
  KbR = :implementation_defined
  KbS = :implementation_defined
  KbT = :implementation_defined
  KbU = :implementation_defined
  KbV = :implementation_defined
  KbW = :implementation_defined
  KbX = :implementation_defined
  KbY = :implementation_defined
  KbZ = :implementation_defined
  KbBackspace = :implementation_defined
  KbDelete = :implementation_defined
  KbDown = :implementation_defined
  KbEnd = :implementation_defined
  # On Numpad
  KbEnter = :implementation_defined
  KbEscape = :implementation_defined
  KbF1 = :implementation_defined
  KbF10 = :implementation_defined
  KbF11 = :implementation_defined
  KbF12 = :implementation_defined
  KbF2 = :implementation_defined
  KbF3 = :implementation_defined
  KbF4 = :implementation_defined
  KbF5 = :implementation_defined
  KbF6 = :implementation_defined
  KbF7 = :implementation_defined
  KbF8 = :implementation_defined
  KbF9 = :implementation_defined
  KbHome = :implementation_defined
  KbInsert = :implementation_defined
  KbLeft = :implementation_defined
  KbLeftAlt = :implementation_defined
  KbLeftControl = :implementation_defined
  KbLeftShift = :implementation_defined
  KbNumpad0 = :implementation_defined
  KbNumpad1 = :implementation_defined
  KbNumpad2 = :implementation_defined
  KbNumpad3 = :implementation_defined
  KbNumpad4 = :implementation_defined
  KbNumpad5 = :implementation_defined
  KbNumpad6 = :implementation_defined
  KbNumpad7 = :implementation_defined
  KbNumpad8 = :implementation_defined
  KbNumpad9 = :implementation_defined
  KbNumpadAdd = :implementation_defined
  KbNumpadDivide = :implementation_defined
  KbNumpadMultiply = :implementation_defined
  KbNumpadSubtract = :implementation_defined
  KbPageDown = :implementation_defined
  KbPageUp = :implementation_defined
  # Above the right shift key
  KbReturn = :implementation_defined
  KbRight = :implementation_defined
  KbRightAlt = :implementation_defined
  KbRightControl = :implementation_defined
  KbRightShift = :implementation_defined
  KbSpace = :implementation_defined
  KbTab = :implementation_defined
  KbUp = :implementation_defined
  MsLeft = :implementation_defined
  MsMiddle = :implementation_defined
  MsRight = :implementation_defined
  MsWheelDown = :implementation_defined
  MsWheelUp = :implementation_defined
  GpButton0 = :implementation_defined
  GpButton1 = :implementation_defined
  GpButton10 = :implementation_defined
  GpButton11 = :implementation_defined
  GpButton12 = :implementation_defined
  GpButton13 = :implementation_defined
  GpButton14 = :implementation_defined
  GpButton15 = :implementation_defined
  GpButton2 = :implementation_defined
  GpButton3 = :implementation_defined
  GpButton4 = :implementation_defined
  GpButton5 = :implementation_defined
  GpButton6 = :implementation_defined
  GpButton7 = :implementation_defined
  GpButton8 = :implementation_defined
  GpButton9 = :implementation_defined
  GpDown = :implementation_defined
  GpLeft = :implementation_defined
  GpRight = :implementation_defined
  GpUp = :implementation_defined

  # Represents an ARGB color value with 8 bits for each channel. Can be
  # replaced by literals of the form 0xaarrggbb in all of Gosu.
  class Color
    attr_accessor :alpha, :red, :green, :blue, :hue, :saturation, :value
    
    # a:: Integer from 0..255
    # r:: Integer from 0..255
    # g:: Integer from 0..255
    # b:: Integer from 0..255
    def initialize(a, r, g, b); end
    # Initializes a color from an 0xrrggbbaa integer.
    def initialize(argb); end
    
    def dup; end
    
    # Same as the constructor, but with an explicit order.
    def self.rgba(r, g, b, a); end    
    
    # Initializes a color from an 0xrrggbbaa integer.
    def self.rgba(rgba); end    
    
    # Same as the constructor, but with an explicit order.
    def self.argb(a, r, g, b); end    
    
    # Initializes a color from an 0xrrggbbaa integer.
    def self.argb(argb); end    
    
    # Converts a HSV triple into a color. Same as from_ahsv with alpha set to 255.
    # h:: Integer from 0..360
    # s:: Float from 0..1
    # v:: Float from 0..1.
    def self.from_hsv(h, s, v); end
    
    # Converts a HSV triple into a color, with a given alpha.
    # a:: Integer from 0..255
    # h:: Integer from 0..360
    # s:: Float from 0..1
    # v:: Float from 0..1.
    def self.from_ahsv(a, h, s, v); end
    
    # 32-bit unsigned value for use with OpenGL ('RGBA' octet in memory).
    def gl; end
    
    # constant
    NONE    = Gosu::Color.argb(0x00000000)
    # constant
    BLACK   = Gosu::Color.argb(0xff000000)
    # constant
    GRAY    = Gosu::Color.argb(0xff808080)
    # constant
    WHITE   = Gosu::Color.argb(0xffffffff)            
    # constant
    AQUA    = Gosu::Color.argb(0xff00ffff)
    # constant
    RED     = Gosu::Color.argb(0xffff0000)
    # constant
    GREEN   = Gosu::Color.argb(0xff00ff00)
    # constant
    BLUE    = Gosu::Color.argb(0xff0000ff)
    # constant
    YELLOW  = Gosu::Color.argb(0xffffff00)
    # constant
    FUCHSIA = Gosu::Color.argb(0xffff00ff)
    # constant
    CYAN    = Gosu::Color.argb(0xff00ffff)
  end
  
  # A font can be used to draw text on a Window object very flexibly.
  # Fonts are ideal for small texts that change regularly. For large,
  # static texts you should use Image#from_text.
  class Font
    attr_reader :name, :height
    
    # font_name:: Name of a system font, or a filename to a TTF file (must contain '/').
    # height:: Height of the font, in pixels.
    def initialize(window, font_name, height); end
    
    # Sets the image to be used for a certain character. Must not be called twice for the same character, or after the character has been drawn already.
    def []=(character, image); end
    
    # Draws text so the top left corner of the text is at (x; y).
    #
    # Characters are created internally as needed.
    def draw(text, x, y, z, factor_x=1, factor_y=1, color=0xffffffff, mode=:default); end
    
    # Draws text at a position relative to (x; y).
    # rel_x:: Determines where the text is drawn horizontally. If relX is 0.0, the text will be to the right of x, if it is 1.0, the text will be to the left of x, if it is 0.5, it will be centered on x. Of course, all real numbers are possible values.
    # rel_y:: See rel_x.
    def draw_rel(text, x, y, z, rel_x, rel_y, factor_x=1, factor_y=1, color=0xffffffff, mode=:default); end
    
    # Returns the width, in pixels, the given text would occupy if drawn.
    def text_width(text, factor_x=1); end
    
    # Analogous to draw, but rotates the text by a given angle.
    # @deprecated Use a combination of Window#rotate and Font#draw instead.
    def draw_rot(text, x, y, z, angle, factor_x=1, factor_y=1, color=0xffffffff, mode=:default); end
  end
  
  # Provides functionality for drawing rectangular images.
  class Image
    attr_reader :width, :height
    
    # Loads an image from a given filename that can be drawn onto the
    # given window. See the Gosu wiki for a list of supported formats.
    #
    # A color key of #ff00ff is automatically applied to BMP type images.
    def initialize(window, filename_or_rmagick_image, tileable); end
    
    # Loads an image from a given filename that can be drawn onto the
    # given window. See the Gosu wiki for a list of supported formats.
    #
    # A color key of #ff00ff is automatically applied to BMP type images.
    #
    # This constructor only loads a sub-rectangle of the given file. Because
    # every call of this constructor will open the image again, it is preferable
    # to use Image#load_tiles when possible.
    def initialize(window, filename_or_rmagick_image, tileable, src_x, src_y, src_width, src_height); end
    
    # Draws the image so its upper left corner is at (x; y).
    def draw(x, y, z, factor_x=1, factor_y=1, color=0xffffffff, mode=:default); end
    
    # center_x:: Relative horizontal position of the rotation center on the image. 0 is the left border, 1 is the right border, 0.5 is the center (and default)
    # center_y:: See center_x.
    def draw_rot(x, y, z, angle, center_x=0.5, center_y=0.5, factor_x=1, factor_y=1, color=0xffffffff, mode=:default); end
    
    # Like Window#draw_quad, but with this texture instead of colors. Can be used to implement advanced, non-rectangular drawing techniques and takes four points and the modulation color at each of them.
    # The points can be in clockwise order, or in a Z shape, starting at the top left.
    def draw_as_quad(x1, y1, c1, x2, y2, c2, x3, y3, c3, x4, y4, c4, z, mode=:default); end
    
    # Creates an Image containing a line of text.
    #
    # The text is always rendered in white. If you want to draw it in a
    # different color, just modulate it by the target color.
    # font_name:: Name of a system font, or a filename to a TTF file (must contain '/').
    # font_height:: Height of the font in pixels.
    def self.from_text(window, text, font_name, font_height); end
    
    # Creates an Image that is filled with the text given to the function.
    #
    # The text may contain line breaks.
    #
    # The text is always rendered in white. If you want to draw it in a
    # different color, just modulate it by the target color.
    # font_name:: Name of a system font, or a filename to a TTF file (must contain '/').
    # font_height:: Height of the font in pixels.
    # line_spacing:: Spacing between two lines of text in pixels.
    # max_width:: Width of the bitmap that will be returned. Text will be split into multiple lines to avoid drawing over the right border. When a single word is too long, it will be truncated.
    # align:: One of :left, :right, :center or :justify.
    def self.from_text(window, text, font_name, font_height, line_spacing, max_width, align); end
    
    # Convenience function that splits an image file into an array of small rectangles and
    # creates images from these. Returns the Array containing Image instances.
    #
    # A color key of #ff00ff is automatically applied to BMP type images.
    #
    # tile_width:: If positive, specifies the width of one tile in pixels. If negative, the bitmap is divided into -tile_width rows.
    # tile_height:: See tile_width.
    def self.load_tiles(window, filename_or_rmagick_image, tile_width, tile_height, tileable); end
    
    # See examples/OpenGLIntegration.rb.
    def gl_tex_info; end
    
    # Returns the associated texture contents as binary string of RGBA values.
    # Useful for use with RMagick (Magick::Image.from_blob).
    def to_blob; end
    
    # Overwrites all or parts of the Image. x and y can be negative or otherwise out of
    # bounds, the incoming image data is clipped to the current image size.
    # This can be used to e.g. overwrite parts of a landscape.
    def insert(filename_or_rmagick_image, x, y); end
    
    # Saves the texture contents as an image file. Useful, for example, to
    # pre-render text on a development machine with proper fonts installed.
    # The file format is determined from the file extension. See the Gosu
    # wiki for a list of portably supported formats.
    def save(filename); end
  end
  
  # A sample is a short sound that is completely loaded in memory, can be
  # played multiple times at once and offers very flexible playback
  # parameters. Use samples for everything that's not music.
  class Sample
    def initialize(window, filename); end
    
    # Plays the sample without panning.
    #
    # Returns a SampleInstance.
    # volume:: Can be anything from 0.0 (silence) to 1.0 (full volume).
    # speed:: Playback speed is only limited by the underlying audio library, and can accept very high or low values. Use 1.0 for normal playback speed.
    def play(vol=1, speed=1, looping=false); end
    
    # Plays the sample with panning. Even if pan is 0.0, the sample will
    # not be as loud as if it were played by calling play() due to the
    # way the panning works.
    #
    # Returns a SampleInstance.
    # volume:: Can be anything from 0.0 (silence) to 1.0 (full volume).
    # speed:: Playback speed is only limited by the underlying audio library, and can accept very high or low values. Use 1.0 for normal playback speed.
    def play_pan(pan=0, vol=1, speed=1, looping=false); end
  end
  
  # An instance of a Sample playing. Can be used to stop sounds dynamically,
  # or to check if they are finished.
  # It is recommended that you throw away sample instances if possible,
  # as they could accidentally refer to other sounds being played after
  # a very long time has passed.
  class SampleInstance
    attr_writer :volume
    attr_writer :speed
    attr_writer :pan
    
    # Stops this instance of a sound being played. Calling this twice, or too late, does not do any harm. You can nil out the reference to the instance afterwards as it will be useless.
    def stop; end
    # Pauses this instance to be resumed afterwards. It will still keep a channel filled while paused.
    def pause; end
    def paused?; end
    def resume; end
    def playing?; end
  end
  
  # Songs are less flexible than samples in that they can only be played
  # one at a time and without panning or speed parameters.
  class Song
    # Returns the song currently being played or paused, or nil if
    # no song has been played yet or the last song has finished
    # playing.
    def self.current_song; end
    
    attr_accessor :volume
    
    def initialize(window, filename); end
    
    # Starts or resumes playback of the song. This will stop all other
    # songs and set the current song to this object.
    def play(looping=false); end
    
    # Pauses playback of the song. It is not considered being played.
    # current_song will stay the same.
    def pause; end
    
    # Returns true if the song is the current song, but in paused
    # mode.
    def paused?; end
    
    # Stops playback of this song if it is currently played or paused.
    # Afterwards, current_song will return 0.
    def stop; end
    
    # Returns true if the song is currently playing.
    def playing?; end
  end
  
  # TextInput instances are invisible objects that build a text string from input,
  # using the current operating system's keyboard layout.
  #
  # At its most basic form, you only need to create a new TextInput instance and
  # pass it to your window via text_input=. Until you call this function again,
  # passing nil, the TextInput object will build a text that can be accessed via
  # TextInput#text.
  #
  # A TextInput object is purely abstract, though; drawing the input field is left
  # to the user. As with most of Gosu, how this is handled is completely left open.
  #
  # TextInput only aims to provide enough code for your own GUIs to build upon.
  class TextInput
    attr_accessor :text
    attr_accessor :caret_pos
    attr_accessor :selection_start
    
    # Overridable filter that is applied to all new text that is entered.
    # Allows for context-sensitive filtering/extending/... of the text.
    # The text will be inserted at caretPos afterwards.
    def filter text_in
      text_in
    end
  end
  
  # Main class that serves as the foundation of a standard
  # Gosu application. Manages initialization of all of Gosu's core components
  # and provides timing functionality.
  #
  # Note that all coordinates, even the mouse position, are in client
  # coordinates relative to the window. This means that the mouse position
  # can be negative or larger than the window size.
  #
  # Note that you should really only use one instance of this class at the same time. This may or may not change later.
  #
  # Right now, having two or more windows and loading samples or songs on both of them will result in an exception.
  class Window
    attr_accessor :caption
    attr_accessor :mouse_x
    attr_accessor :mouse_y
    attr_accessor :text_input
    attr_reader :width, :height
    attr_reader :fullscreen?
    attr_reader :update_interval
    
    # update_interval:: Interval in milliseconds between two calls
    # to the update member function. The default means the game will run
    # at 60 FPS, which is ideal on standard 60 Hz TFT screens.
    def initialize(width, height, fullscreen, update_interval=16.666666); end
    
    # Enters a modal loop where the Window is visible on screen and receives calls to draw, update etc.
    def show; end
    
    # Tells the window to end the current show loop as soon as possible.
    def close; end 
    
    # Called every update_interval milliseconds while the window is being
    # shown. Your application's main game logic goes here.
    def update; end
    
    # Called after every update and when the OS wants the window to
    # repaint itself. Your application's rendering code goes here.
    def draw; end
    
    # Can be overriden to give the game a chance to say no to being redrawn.
    # This is not a definitive answer. The operating system can still cause
    # redraws for one reason or another.
    #
    # By default, the window is redrawn all the time (i.e. Window#needs_redraw?
    # always returns true.)
    def needs_redraw?; end
    
    # Can be overriden to show the system cursor when necessary, e.g. in level
    # editors or other situations where introducing a custom cursor is not
    # desired.
    def needs_cursor?; end
    
    # Called before update when the user pressed a button while the
    # window had the focus.
    def button_down(id); end
    # Same as buttonDown. Called then the user released a button.
    def button_up(id); end
    
    # Returns true if a button is currently pressed. Updated every tick.
    def button_down?(id); end
    
    # Draws a line from one point to another (last pixel exclusive).
    # Note: OpenGL lines are not reliable at all and may have a missing pixel at the start
    # or end point. Please only use this for debugging purposes. Otherwise, use a quad or
    # image to simulate lines, or contribute a better draw_line to Gosu.
    def draw_line(x1, y1, c1, x2, y2, c2, z=0, mode=:default); end
    
    def draw_triangle(x1, y1, c1, x2, y2, c2, x3, y3, c3, z=0, mode=:default); end
    
    # Draws a rectangle (two triangles) with given corners and corresponding
    # colors.
    # The points can be in clockwise order, or in a Z shape.
    def draw_quad(x1, y1, c1, x2, y2, c2, x3, y3, c3, x4, y4, c4, z=0, mode=:default); end
    
    # Flushes all drawing operations to OpenGL so that Z-ordering can start anew. This
    # is useful when drawing several parts of code on top of each other that use conflicting
    # z positions.
    def flush; end
    
    # For custom OpenGL calls. Executes the given block in a clean OpenGL environment.
    # Use the ruby-opengl gem to access OpenGL function (if you manage to get it to work).
    # IF no z position is given, it will execute the given block immediately, otherwise,
    # the code will be scheduled to be called between Gosu drawing operations.
    #
    # Note: You cannot call Gosu rendering functions within this block, and you can only
    # call the gl function in the call tree of Window#draw.
    #
    # See examples/OpenGLIntegration.rb for an example.
    def gl(z=nil, &custom_gl_code); end
    
    # Limits the drawing area to a given rectangle while evaluating the code inside of the block.
    def clip_to(x, y, w, h, &rendering_code); end
    
    # Returns a Gosu::Image that containes everything rendered within the given block. It can be
    # used to optimize rendering of many static images, e.g. the map. There are still several
    # restrictions that you will be informed about via exceptions.
    #
    # The returned Gosu::Image will have the width and height you pass as arguments, regardless
    # of how the area you draw on. It is important to pass accurate values if you plan on using
    # Gosu::Image#draw_as_quad or Gosu::Image#draw_rot with the result later.
    #
    # @return [Gosu::Image]
    def record(width, height, &rendering_code); end
    
    # Rotates everything drawn in the block around (around_x, around_y).
    def rotate(angle, around_x=0, around_y=0, &rendering_code); end
    
    # Scales everything drawn in the block by a factor.
    def scale(factor_x, factor_y=factor_x, &rendering_code); end
    
    # Scales everything drawn in the block by a factor for each dimension.
    def scale(factor_x, factor_y, around_x, around_y, &rendering_code); end
    
    # Moves everything drawn in the block by an offset in each dimension.
    def translate(x, y, &rendering_code); end
    
    # Applies a free-form matrix rotation to everything drawn in the block.
    def transform(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, &rendering_code); end
    
    # Returns the character a button usually produces, or nil. To implement real text-input
    # facilities, look at the TextInput class instead.
    def self.button_id_to_char(id); end
    
    # Returns the button that has to be pressed to produce the given character, or nil.
    def self.char_to_button_id(char); end
    
    # @deprecated Use Window#mouse_x= and Window#mouse_y= instead.
    def set_mouse_position(x, y); end
  end
  
  # Contains information about the underlying OpenGL texture and the u/v space used for image data.
  #
  # Can be retrieved from some images to use them in OpenGL operations. nil will be returned instead by images that are too large for a single texture.)
  #
  # See examples/OpenGLIntegration.rb.
  class GLTexInfo
    attr_reader :tex_name, :left, :right, :top, :bottom
  end
  
  # Returns a random double between min (inclusive) and max (exclusive).
  def random(min, max); end
  
  # Returns the horizontal distance between the origin and the point to which you would get if you moved radius pixels in the direction specified by angle.
  def offset_x(angle, dist); end 
  
  # Returns the vertical distance between the origin and the point to which you would get if you moved radius pixels in the direction specified by angle.
  def offset_y(angle, dist); end
  
  # Returns the angle from point 1 to point 2 in degrees, where 0.0 means upwards. Returns 0 if both points are equal.
  def angle(x1, y1, x2, y2); end
  
  # Returns the smallest angle that can be added to angle1 to get to angle2 (can be negative if counter-clockwise movement is shorter).
  def angle_diff(angle1, angle2); end
  
  # Returns the distance between two points.
  def distance(x1, y1, x2, y2); end
  
  # Incrementing, possibly wrapping millisecond timer.
  def milliseconds(); end
  
  # Returns the name of a neutral font that is available on the current
  # platform.
  def default_font_name(); end
  
  # Returns the width, in pixels, of the user's primary screen.
  def screen_width(); end
  
  # Returns the height, in pixels, of the user's primary screen.
  def screen_height(); end
  
  # Returns the user's preferred language, at the moment of calling the function. Expect return
  # values such as 'en_US', 'de_DE.UTF-8', 'ja', 'zh-Hans'. You can rely only on the first two letters
  # being a common language abbreviation.
  def language(); end
end

# Small additions to Numeric to make it easier to integrate Gosu with
# libraries that use radians, like Chipmunk.
class ::Numeric
  # Returns <tt>self * 180.0 / Math::PI + 90</tt>.
  # Translates between Gosu's angle system (where 0° is at the top) and
  # radians (where 0 is at the right).
  def radians_to_gosu(); end

  # Returns <tt>(self - 90) * Math::PI / 180.0</tt>
  # Translates between Gosu's angle system (where 0° is at the top) and
  # radians (where 0 is at the right).
  def gosu_to_radians(); end
  
  # Scales a degree value to radians.
  def degrees_to_radians(); end
  
  # Scales a radian value to degrees.
  def radians_to_degrees(); end
end
