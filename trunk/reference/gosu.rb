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
  KbPause = :implementation_defined
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

    # argb: 0xAARRGGBB integer.
  	def initialize(argb); end
  	# a:: Integer from 0..255
  	# r:: Integer from 0..255
  	# g:: Integer from 0..255
  	# b:: Integer from 0..255
    def initialize(a, r, g, b); end
    
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
  end
  
  # A font can be used to draw text on a Window object very flexibly.
  # Fonts are ideal for small texts that change regularly. For large,
  # static texts you should use Image#from_text.
  class Font
    attr_reader :height
    
    # font_name:: Name of a system font, or a filename to a TTF file (must contain '/', does not work on Linux).
    # height:: Height of the font, in pixels.
    def initialize(window, font_name, height); end
    
    # Returns the width, in pixels, the given text would occupy if drawn.
    def text_width(text, factor_x=1); end
    
    # Draws text so the top left corner of the text is at (x; y).
    #
    # Characters are created internally as needed.
    def draw(text, x, y, z, factor_x=1, factor_y=1, color=0xffffffff, mode=:default); end
    
    # Draws text at a position relative to (x; y).
    # rel_x:: Determines where the text is drawn horizontally. If relX is 0.0, the text will be to the right of x, if it is 1.0, the text will be to the left of x, if it is 0.5, it will be centered on x. Of course, all real numbers are possible values.
    # rel_y:: See rel_x.
    def draw_rel(text, x, y, z, rel_x, rel_y, factor_x=1, factor_y=1, color=0xffffffff, mode=:default); end
      
    # Analogous to draw, but rotates the text by a given angle.
    def draw_rot(text, x, y, z, angle, factor_x=1, factor_y=1, color=0xffffffff, mode=:default); end
  end
  
  # Provides functionality for drawing rectangular images.
  class Image
    attr_reader :width, :height
    
    # Loads an image from a given filename that can be drawn onto
    # the given window.
    #
		# This constructor can handle PNG and BMP images. A color key of #ff00ff is
		# automatically applied to BMP type images. For more flexibility, use PNG
		# files.
    def initialize(window, filename_or_rmagick_image, tileable); end

    # Loads an image from a given filename that can be drawn onto
    # the given window.
    #
		# This constructor can handle PNG and BMP images. A color key of #ff00ff is
		# automatically applied to BMP type images. For more flexibility, use PNG
		# files.
		#
		# This constructor only loads a sub-rectangle of the given file. Because
		# every call of this constructor will open the image again, it is preferable
		# to use Image#load_tiles.
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
    # font_name:: Name of a system font, or a filename to a TTF file (must contain '/', does not work on Linux).
    # font_height:: Height of the font in pixels.
    def self.from_text(window, text, font_name, font_height); end

    # Creates an Image that is filled with the text given to the function.
    #
    # The text may contain line breaks.
    #
    # The text is always rendered in white. If you want to draw it in a
    # different color, just modulate it by the target color.
    # font_name:: Name of a system font, or a filename to a TTF file (must contain '/', does not work on Linux).
    # font_height:: Height of the font in pixels.
    # line_spacing:: Spacing between two lines of text in pixels.
    # max_width:: Width of the bitmap that will be returned. Text will be split into multiple lines to avoid drawing over the right border. When a single word is too long, it will be truncated.
    # align:: One of :left, :right, :center or :justify.
    # enum.
    def self.from_text(window, text, font_name, font_height, line_spacing, max_width, align); end
    
    # Convenience function that splits a BMP or PNG file into an array
    # of small rectangles and creates images from them.
    # Returns the Array containing Image instances.
    # tile_width:: If positive, specifies the width of one tile in pixels. If negative, the bitmap is divided into -tile_width rows.
    # tile_height:: See tile_width.
    def self.load_tiles(window, filename_or_rmagick_image, tile_width, tile_height, tileable); end
    
    # See examples/OpenGLIntegration.rb.
    def gl_tex_info; end
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
    attr_accessor :volume
    attr_accessor :speed
    attr_accessor :pan
    
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
    def play(looping = false); end
    
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
    attr_reader :caret_pos
    attr_reader :selection_start
  end
  
  # Main class that serves as the foundation of a standard
	# Gosu application. Manages initialization of all of Gosu's core components
  # and provides timing functionality.
  #
  # Note that all coordinates, even the mouse position, are in client
  # coordinates relative to the window. This means that the mouse position
  # can be negative or larger than the window size.
  #
  # Note that you should really only use on instance of this class at the same time. This may or may not change later.
  #
  # Right now, having two or more windows and loading samples or songs on both of them will result in an exception. If you want to re-open your game window, make sure the old one is really dead, e.g. by poking the garbage collector.
  class Window
    attr_accessor :caption
    attr_accessor :mouse_x
    attr_accessor :mouse_y
    attr_accessor :text_input
    attr_reader :update_interval
    attr_reader :width, :height
    attr_reader :update_interval
    
    # update_interval:: Interval in milliseconds between two calls
    # to the update member function. The default means the game will run
    # at 60 FPS, which is ideal on standard 60 Hz TFT screens.
    def initialize(width, height, fullscreen, update_interval = 16.666666); end
    
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
    
    # Gives the game a chance to say no to being redrawn.
    # This is not a definitive answer. The operating system can still cause
    # redraws for one reason or another.
    # By default, the window is redrawn all the time.
    def needs_redraw?; end
    
    # To avoid the intermediate position of calling mouse_x= followed by mouse_y=.
    def set_mouse_position(x, y); end
    
    # Called before update when the user pressed a button while the
    # window had the focus.
    def button_down(id); end
    # Same as buttonDown. Called then the user released a button.
    def button_up(id); end
    
    # Draws a line from one point to another (last pixel exclusive).
    def draw_line(x1, y1, c1, x2, y2, c2, z=0, mode=:default); end
      
    def draw_triangle(x1, y1, c1, x2, y2, c2, x3, y3, c3, z=0, mode=:default); end
    
    # Draws a rectangle (two triangles) with given corners and corresponding
    # colors.
    # The points can be in clockwise order, or in a Z shape.
    def draw_quad(x1, y1, c1, x2, y2, c2, x3, y3, c3, x4, y4, c4, z=0, mode=:default); end
    
  	# Returns true if a button is currently pressed. Updated every tick.
    def button_down?(id); end
    
    # Returns the character a button usually produces, or ni.
    def self.button_id_to_char(id); end
    
    # Returns the button that has to be pressed to produce the
    # given character, or nil.
    def self.char_to_button_id(char); end
    
    # See examples/OpenGLIntegration.rb.
    def gl(&custom_gl_code); end
    
    # Limits the drawing area to a given rectangle while evaluating the code inside of the block.
    def clip_to(x, y, w, h, &drawing_code); end
  end
  
  # Contains information about the underlying OpenGL texture and the u/v space used for image data.
  #
  # Can be retrieved from some images to use them in OpenGL operations. (Will not work with images that are too large for a single texture.)
  #
  # See examples/OpenGLIntegration.rb.
  class GLTexInfo
    attr_accessor :tex_name, :left, :right, :top, :bottom
  end
  
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
end

# Small additions to Numeric to make it easier to integrate Gosu with
# libraries that use radians, like Chipmunk.
class ::Numeric
  # Returns <tt>self * 180.0 / Math::PI + 90</tt>
  def radians_to_gosu(); end

  # Returns <tt>(self - 90) * Math::PI / 180.0</tt>
  def gosu_to_radians(); end
end
