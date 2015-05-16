##
# The first component of the version.
GOSU_MAJOR_VERSION = :a_fixnum

##
# The second component of the version.
GOSU_MINOR_VERSION = :a_fixnum

##
# The third component of the version.
GOSU_POINT_VERSION = :a_fixnum

##
# A version string of the form "0.1.2", "0.1.2.3" or "0.1.2pre4".
GOSU_VERSION = :a_string

##
# A long block of legal copy that your game is obliged to display somewhere.
GOSU_COPYRIGHT_NOTICE = :a_string

module Gosu
  Kb0 Kb1 Kb2 Kb3 Kb4 Kb5 Kb6 Kb7 Kb8 Kb9 = :implementation_defined
  KbA KbB KbC KbD KbE KbF KbG KbH KbI KbJ KbK KbL KbM KbN KbO KbP KbQ KbR KbS KbT KbU KbV KbW KbX KbY KbZ = :implementation_defined
  KbBackspace = :implementation_defined
  KbDelete = :implementation_defined
  KbDown = :implementation_defined
  KbEnd = :implementation_defined
  
  ##
  # This is the key on the numpad.
  KbEnter = :implementation_defined
  
  KbEscape = :implementation_defined
  KbF1 KbF2 KbF3 KbF4 KbF5 KbF6 KbF7 KbF8 KbF9 KbF10 KbF11 KbF12 = :implementation_defined
  KbHome = :implementation_defined
  KbInsert = :implementation_defined
  KbLeft = :implementation_defined
  KbLeftAlt = :implementation_defined
  KbLeftControl = :implementation_defined
  KbLeftShift = :implementation_defined
  KbNumpad0 KbNumpad1 KbNumpad2 KbNumpad3 KbNumpad4 KbNumpad5 KbNumpad6 KbNumpad7 KbNumpad8 KbNumpad9 = :implementation_defined
  KbNumpadAdd = :implementation_defined
  KbNumpadDivide = :implementation_defined
  KbNumpadMultiply = :implementation_defined
  KbNumpadSubtract = :implementation_defined
  KbPageDown = :implementation_defined
  KbPageUp = :implementation_defined
  
  ##
  # This is the key above the right shift key.
  KbReturn = :implementation_defined
  
  KbRight = :implementation_defined
  KbRightAlt = :implementation_defined
  KbRightControl = :implementation_defined
  KbRightShift = :implementation_defined
  KbSpace = :implementation_defined
  KbTab = :implementation_defined
  KbUp = :implementation_defined
  KbBacktick = :implementation_defined
  KbMinus = :implementation_defined
  KbEqual = :implementation_defined
  KbBracketLeft = :implementation_defined
  KbBracketRight = :implementation_defined
  KbBackslash = :implementation_defined
  KbSemicolon = :implementation_defined
  KbApostrophe = :implementation_defined
  KbComma = :implementation_defined
  KbPeriod = :implementation_defined
  KbSlash = :implementation_defined
  
  MsLeft = :implementation_defined
  MsMiddle = :implementation_defined
  MsRight = :implementation_defined
  MsWheelDown = :implementation_defined
  MsWheelUp = :implementation_defined
  MsOther0 MsOther1 MsOther2 MsOther3 MsOther4 MsOther5 MsOther6 MsOther7 = :implementation_defined
  
  GpButton0 GpButton1 GpButton2 GpButton3 GpButton4 GpButton5 GpButton6 GpButton7 GpButton8 GpButton9 GpButton10 GpButton11 GpButton12 GpButton13 GpButton14 GpButton15 = :implementation_defined
  GpDown = :implementation_defined
  GpLeft = :implementation_defined
  GpRight = :implementation_defined
  GpUp = :implementation_defined
  
  Gp0Button0 Gp0Button1 Gp0Button2 Gp0Button3 Gp0Button4 Gp0Button5 Gp0Button6 Gp0Button7 Gp0Button8 Gp0Button9 Gp0Button10 Gp0Button11 Gp0Button12 Gp0Button13 Gp0Button14 Gp0Button15 = :implementation_defined
  Gp0Down = :implementation_defined
  Gp0Left = :implementation_defined
  Gp0Right = :implementation_defined
  Gp0Up = :implementation_defined
  
  Gp1Button0 Gp1Button1 Gp1Button2 Gp1Button3 Gp1Button4 Gp1Button5 Gp1Button6 Gp1Button7 Gp1Button8 Gp1Button9 Gp1Button10 Gp1Button11 Gp1Button12 Gp1Button13 Gp1Button14 Gp1Button15 = :implementation_defined
  Gp1Down = :implementation_defined
  Gp1Left = :implementation_defined
  Gp1Right = :implementation_defined
  Gp1Up = :implementation_defined
  
  Gp2Button0 Gp2Button1 Gp2Button2 Gp2Button3 Gp2Button4 Gp2Button5 Gp2Button6 Gp2Button7 Gp2Button8 Gp2Button9 Gp2Button10 Gp2Button11 Gp2Button12 Gp2Button13 Gp2Button14 Gp2Button15 = :implementation_defined
  Gp2Down = :implementation_defined
  Gp2Left = :implementation_defined
  Gp2Right = :implementation_defined
  Gp2Up = :implementation_defined
  
  Gp3Button0 Gp3Button1 Gp3Button2 Gp3Button3 Gp3Button4 Gp3Button5 Gp3Button6 Gp3Button7 Gp3Button8 Gp3Button9 Gp3Button10 Gp3Button11 Gp3Button12 Gp3Button13 Gp3Button14 Gp3Button15 = :implementation_defined
  Gp3Down = :implementation_defined
  Gp3Left = :implementation_defined
  Gp3Right = :implementation_defined
  Gp3Up = :implementation_defined
  
  ##
  # Represents an ARGB color value with 8 bits for each channel. Colors can be used interchangeably with integer literals of the form 0xAARRGGBB in all Gosu APIs.
  class Color
    ##
    # @return [Fixnum] the color's alpha channel.
    attr_accessor :alpha
    
    ##
    # @return [Fixnum] the color's red channel.
    attr_accessor :red
    
    ##
    # @return [Fixnum] the color's green channel.
    attr_accessor :green
    
    ##
    # @return [Fixnum] the color's blue channel.
    attr_accessor :blue
    
    ##
    # @return [Fixnum] the color's hue in the range (0...360).
    attr_accessor :hue
    
    ##
    # @return [Float] the color's saturation in the range (0..1).
    attr_accessor :saturation
    
    ##
    # @return [Float] the color's value in the range (0..1).
    attr_accessor :value
    
    # @!group Creating colors.
    
    ##
    # @overload initialize(argb)
    #   @param argb [Fixnum] an integer of the form 0xAARRGGBB.
    # 
    # @overload initialize(a, r, g, b)
    #   @param a [Fixnum] the color's alpha channel in the range (0..255).
    #   @param r [Fixnum] the color's red channel in the range (0..255).
    #   @param g [Fixnum] the color's green channel in the range (0..255).
    #   @param b [Fixnum] the color's blue channel in the range (0..255).
    # 
    # @see from_hsv
    # @see from_ahsv
    # @see rgba
    # @see argb
    def initialize(*args); end
    
    ##
    # @return (see #initialize)
    # 
    # @overload rgba(rgba)
    #   @param argb [Fixnum] an integer of the form 0xRRGGBBAA.
    # 
    # @overload rgba(r, g, b, a)
    #   @param r [Fixnum] the color's red channel in the range (0..255).
    #   @param g [Fixnum] the color's green channel in the range (0..255).
    #   @param b [Fixnum] the color's blue channel in the range (0..255).
    #   @param a [Fixnum] the color's alpha channel in the range (0..255).
    # 
    # @see #initialize
    # @see argb
    def self.rgba(*args); end
    
    # This method is equivalent to calling `Color.new`, but the name makes the parameter order explicit.
    # 
    # @return (see #initialize)
    # @overload argb(argb)
    # @overload argb(a, r, g, b)
    # 
    # @see #initialize
    # @see rgba
    def self.argb(*args); end
    
    # Converts an HSV triplet to an opaque color.
    # 
    # @return [Color] a color corresponding to the HSV triplet.
    # @param h [Fixnum] the color's hue in the range (0..360).
    # @param s [Float] the color's saturation in the range (0..1).
    # @param v [Float] the color's value in the range (0..1).
    # 
    # @see from_ahsv
    def self.from_hsv(h, s, v); end
    
    # Converts an HSV triplet to a color with the alpha channel set to a given value.
    # 
    # @return (see from_hsv)
    # @param a [Fixnum] the color's opacity in the range (0..255).
    # @param (see from_hsv)
    # 
    # @see from_hsv
    def self.from_ahsv(a, h, s, v); end
    
    # @!endgroup
    
    # Returns a 32-bit representation of the color suitable for use with OpenGL calls. This color is stored in a fixed order in memory and its integer value may vary depending on your system's byte order.
    # 
    # @return [Fixnum] a 32-bit OpenGL color.
    def gl; end
    
    ##
    # @return [Color] a copy of the color.
    def dup; end
    
    NONE    = Gosu::Color.argb(0x00_000000)
    BLACK   = Gosu::Color.argb(0xff_000000)
    GRAY    = Gosu::Color.argb(0xff_808080)
    WHITE   = Gosu::Color.argb(0xff_ffffff)
    AQUA    = Gosu::Color.argb(0xff_00ffff)
    RED     = Gosu::Color.argb(0xff_ff0000)
    GREEN   = Gosu::Color.argb(0xff_00ff00)
    BLUE    = Gosu::Color.argb(0xff_0000ff)
    YELLOW  = Gosu::Color.argb(0xff_ffff00)
    FUCHSIA = Gosu::Color.argb(0xff_ff00ff)
    CYAN    = Gosu::Color.argb(0xff_00ffff)
  end
  
  ##
  # A Font can be used to draw text on a Window object very flexibly.
  # Fonts are ideal for small texts that change regularly.
  # For large, static texts you should use {Gosu::Image#from_text}.
  class Font
    ##
    # The font's name. This may be the name of a system font or a filename.
    # 
    # @return [String] the font's name.
    attr_reader :name
    
    ##
    # @return [Fixnum] the font's height in pixels.
    attr_reader :height
    
    ##
    # Load a font from the system fonts or a file.
    # 
    # (Passing a Window reference is not necessary anymore, please use the first overload from now on.)
    # 
    # @param height [Fixnum] the height of the font, in pixels.
    # @param [Hash] options
    # @option options [String] :name the name of a system font, or a path to a TrueType Font (TTF) file. A path must contain at least one '/' character to distinguish it from a system font.
    # 
    # @overload initialize(height, options = {})
    # @overload initialize(window, font_name, height)
    def initialize(height, options = {}); end
    
    ##
    # Overrides the image for a character.
    # 
    # @note For any given character, this method MUST NOT be called more than once, and MUST NOT be called if a string containing the character has already been drawn.
    # 
    # @return [void]
    # @param character [String] the character to replace.
    # @param image [Image] the image to use for the character.
    def []=(character, image); end
    
    # @!group Drawing text
    
    ##
    # Draws a single line of text with its top left corner at (x, y).
    # 
    # @return [void]
    # @param text [String]
    # @param x [Number] the X coordinate
    # @param y [Number] the Y coordinate
    # @param z [Number] the Z-order.
    # @param scale_x [Float] the horizontal scaling factor.
    # @param scale_y [Float] the vertical scaling factor.
    # @param color [Color, Fixnum]
    # @param mode [:default, :additive] the blending mode to use.
    # 
    # @see #draw_rel
    # @see Gosu::Image.from_text
    # @see https://github.com/jlnr/gosu/wiki/Basic-Concepts#drawing-with-colours Drawing with colors, explained in the Gosu Wiki
    # @see https://github.com/jlnr/gosu/wiki/Basic-Concepts#z-ordering Z-ordering explained in the Gosu Wiki
    def draw(text, x, y, z, scale_x=1, scale_y=1, color=0xff_ffffff, mode=:default); end
    
    ##
    # Draws a single line of text relative to (x, y).
    # 
    # The text is aligned to the drawing location according to the `rel_x` and `rel_y` parameters: a value of 0.0 corresponds to top and left, while 1.0 corresponds to bottom and right. A value of 0.5 naturally corresponds to the center of the text.
    # 
    # All real numbers are valid alignment values and will be interpolated (or extrapolated) accordingly.
    # 
    # @return [void]
    # @param rel_x [Float] the horizontal alignment.
    # @param rel_y [Float] the vertical alignment.
    # @param (see #draw)
    # 
    # @see #draw
    # @see https://github.com/jlnr/gosu/wiki/Basic-Concepts#drawing-with-colours Drawing with colors, explained in the Gosu Wiki
    # @see https://github.com/jlnr/gosu/wiki/Basic-Concepts#z-ordering Z-ordering explained in the Gosu Wiki
    def draw_rel(text, x, y, z, rel_x, rel_y, scale_x=1, scale_y=1, color=0xff_ffffff, mode=:default); end
    
    ##
    # @deprecated Use {#draw} in conjunction with {Gosu.rotate} instead.
    # 
    # @see #draw
    # @see Gosu::Window#rotate
    def draw_rot(text, x, y, z, angle, scale_x=1, scale_y=1, color=0xff_ffffff, mode=:default); end
    
    # @!endgroup
    
    ##
    # Returns the width of a single line of text, in pixels, if it were drawn.
    # 
    # @return [Fixnum] the width of the text, in pixels.
    # @param text [String]
    def text_width(text, scale_x=1); end
  end
  
  ##
  # Provides functionality for drawing rectangular images.
  class Image
    ##
    # @return [Fixnum] the image's width, in pixels.
    attr_reader :width
    
    ##
    # @return [Fixnum] the image's height, in pixels.
    attr_reader :height
    
    # @!group Creating and loading images
    
    ##
    # Loads an image from a file or an RMagick image.
    # 
    # (Passing a Window reference is not necessary anymore, please use the first overload from now on.)
    # 
    # @note For Windows Bitmap (BMP) images, magenta (FF00FF, often called "magic pink" in this context) is treated as a chroma key and all pixels of that color are automatically rendered fully transparent.
    # 
    # @param [String, Magick::Image] source the filename or RMagick image to load from.
    # @param [Hash] options
    # @option options [true, false] :tileable (false) if true, the Image will not have soft edges when scaled
    # @option options [Array] :rect ([0, 0, image_width, image_height]) the source rectangle in the image
    # 
    # @overload initialize(source, options = {})
    # @overload initialize(window, source, tileable = false)
    # @overload initialize(window, source, tileable, left, top, width, height)
    # 
    # @see load_tiles
    # @see from_text
    # @see https://github.com/jlnr/gosu/wiki/Basic-Concepts#tileability Tileability explained in the Gosu Wiki
    def initialize(source, options = {}); end
    
    ##
    # Creates a reusable image from one or more lines of text.
    #
    # (Passing a Window reference is not necessary anymore, please use the first overload from now on.)
    # 
    # @note The text is always rendered in white. To draw it in a different color, use the color parameter of {#draw}, et al.
    # 
    # @overload from_text(text, line_height, options = {})
    # @overload from_text(window, text, font_name, line_height)
    # @overload from_text(window, text, font_name, line_height, line_spacing, width, align)
    # 
    # @return [Gosu::Image]
    # @param [String] text
    # @param [Fixnum] line_height the line height, in pixels.
    # @param [Hash] options
    # @option options [String] :font (Gosu::default_font_name) the name of a system font, or a path to a TrueType Font (TTF) file. A path must contain at least one '/' character to distinguish it from a system font.
    # @option options [Fixnum] :width the width of the image, in pixels. Long lines will be automatically wrapped around to avoid overflow, but overlong words will be truncated. If this option is omitted, lines will not be wrapped, and :align and :spacing will be ignored as well.
    # @option options [Fixnum] :spacing (0) the spacing between lines, in pixels.
    # @option options [:left, :right, :center, :justify] :align (:left) the text alignment.
    # 
    # @see Gosu::Font
    # @see https://github.com/jlnr/gosu/wiki/Basic-Concepts#drawing-with-colours Drawing with colors, explained in the Gosu Wiki
    def self.from_text(text, line_height, options = {}); end
    
    ##
    # Loads an image from a file or an RMagick image, then divides the image into an array of equal-sized tiles.
    # 
    # @note For Windows Bitmap (BMP) images, magenta (FF00FF, often called "magic pink" in this context) is treated as a chroma key and all pixels of that color are automatically rendered fully transparent.
    #
    # @return [Array<Gosu::Image>]
    # @param source [String, Magick::Image]
    # @param tile_width [Fixnum] If positive, this is the width of the individual tiles; if negative, the image is divided into -tile_width columns.
    # @param tile_height [Fixnum] If positive, this is the height of the individual tiles; if negative, the image is divided into -tile_height rows.
    # @param [Hash] options
    # @option options [true, false] :tileable (false) if true, the Image will not have soft edges when scaled
    # 
    # @overload initialize(source, tile_width, tile_height, options = {})
    # @overload initialize(window, source, tile_width, tile_height, tileable = false)
    # 
    # (Passing a Window reference is not necessary anymore, please use the first overload from now on.)
    # 
    # @see https://github.com/jlnr/gosu/wiki/Basic-Concepts#tileability Tileability explained in the Gosu Wiki
    def self.load_tiles(source, tile_width, tile_height, options = {}); end
    
    # @!endgroup
    
    # @!group Drawing an image
    
    ##
    # Draws the image with its top left corner at (x, y).
    # 
    # @return [void]
    # @param x [Float] the X coordinate.
    # @param y [Float] the X coordinate.
    # @param z [Float] the Z-order.
    # @param scale_x [Float] the horizontal scaling factor.
    # @param scale_y [Float] the vertical scaling factor.
    # @param color [Gosu::Color, Integer]
    # @param mode [:default, :additive] the blending mode to use.
    # 
    # @see #draw_rot
    # @see #draw_as_quad
    # @see https://github.com/jlnr/gosu/wiki/Basic-Concepts#drawing-with-colours Drawing with colors, explained in the Gosu Wiki
    # @see https://github.com/jlnr/gosu/wiki/Basic-Concepts#z-ordering Z-ordering explained in the Gosu Wiki
    def draw(x, y, z, scale_x=1, scale_y=1, color=0xff_ffffff, mode=:default); end
    
    ##
    # Draws the image rotated, with its rotational center at (x, y).
    # 
    # @return [void]
    # @param angle [Float]
    # @param center_x [Float] the relative horizontal rotation origin.
    # @param center_y [Float] the relative vertical rotation origin.
    # @param (see #draw)
    # 
    # @see #draw
    # @see https://github.com/jlnr/gosu/wiki/Basic-Concepts#drawing-with-colours Drawing with colors, explained in the Gosu Wiki
    # @see https://github.com/jlnr/gosu/wiki/Basic-Concepts#z-ordering Z-ordering explained in the Gosu Wiki
    def draw_rot(x, y, z, angle, center_x=0.5, center_y=0.5, scale_x=1, scale_y=1, color=0xff_ffffff, mode=:default); end
    
    ##
    # Draws the image as an arbitrary quad. This method can be used for advanced non-rectangular drawing techniques, e.g., faking perspective or isometric projection.
    # 
    # @return [void]
    # @param (see Gosu::Window#draw_quad)
    # 
    # @see #draw
    # @see Gosu::Window#draw_quad
    # @see https://github.com/jlnr/gosu/wiki/Basic-Concepts#drawing-with-colours Drawing with colors, explained in the Gosu Wiki
    # @see https://github.com/jlnr/gosu/wiki/Basic-Concepts#order-of-corners The order of corners explained in the Gosu Wiki
    # @see https://github.com/jlnr/gosu/wiki/Basic-Concepts#z-ordering Z-ordering explained in the Gosu Wiki
    def draw_as_quad(x1, y1, c1, x2, y2, c2, x3, y3, c3, x4, y4, c4, z, mode=:default); end
    
    # @!endgroup
    
    ##
    # Returns an object that holds information about the underlying OpenGL texture and UV coordinates of the image.
    # 
    # @note Some images may be too large to fit on a single texture; this method returns nil in those cases.
    # 
    # @return [Gosu::GLTexInfo?] information about the underlying OpenGL texture.
    # 
    # @see Gosu::GLTexInfo
    # @see file:examples/OpenGLIntegration.rb
    def gl_tex_info; end
    
    ##
    # Returns the associated texture contents as binary string of packed RGBA values, useful for use with RMagick (Magick::Image.from_blob).
    # 
    # @return [String] a binary string of packed RGBA values.
    def to_blob; end
    
    ##
    # Overwrites part of the image with the contents of another. If the source image is partially out of bounds, it will be clipped to fit.
    # 
    # This can be used to e.g. overwrite parts of a landscape.
    # 
    # @return [void]
    # @param source [String, Magick::Image] the filename or RMagick image to load from.
    # @param x [Fixnum] the X coordinate of the top left corner.
    # @param y [Fixnum] the Y coordinate of the top left corner.
    def insert(source, x, y); end
    
    ##
    # Saves the image to a file. The file format is determined from the file extension.
    # 
    # Useful for, e.g., pre-rendering text on a development machine where the necessary fonts are known to be available.
    # 
    # @return [void]
    # @param filename [String] the path to save the file under.
    def save(filename); end
  end
  
  ##
  # A sample is a short sound that is completely loaded in memory, can be
  # played multiple times at once and offers very flexible playback
  # parameters. Use samples for everything that's not music.
  # 
  # @see Gosu::Song
  class Sample
    ##
    # Loads a sample from a file.
    # 
    # (Passing a Window reference is not necessary anymore, please use the first overload from now on.)
    # 
    # @overload initialize(filename)
    # @overload initialize(window, filename)
    #
    # @param filename [String] the path to load the sample from.
    def initialize(filename); end
    
    ##
    # Plays the sample without panning.
    # 
    # Playback speed is limited only by the underlying audio library, and both very large and very small values should work just fine.
    #
    # @return [SampleInstance]
    # @param volume [Float] the playback volume, in the range (0..1), where 0 is completely silent and 1 is full volume.
    # @param speed [Float] the playback speed.
    # @param looping [true, false] whether the sample should play in a loop.
    # 
    # @see #play_pan
    def play(volume=1, speed=1, looping=false); end
    
    ##
    # Plays the sample with panning.
    # 
    # @note Samples played with this method will not be as loud as those played with {#play}, even if `pan` is 0. This is due to a limitation in the way panning works.
    #
    # @return [SampleInstance]
    # @param pan [Float] the amount of panning. 0 is centered.
    # @param (see #play)
    # 
    # @see #play
    def play_pan(pan=0, volume=1, speed=1, looping=false); end
  end
  
  ##
  # An instance of a {Gosu::Sample} playing. Can be used to stop sounds dynamically, or to check if they are finished.
  # 
  # It is recommended to throw away sample instances as soon as possible, as holding onto them for a long time can prevent unneeded samples being properly disposed.
  class SampleInstance
    attr_writer :volume
    attr_writer :speed
    attr_writer :pan
    
    ##
    # Stops playback of this sample instance. After calling this method, the sample instance is useless and can be discarded.
    # 
    # Calling `stop` after the sample has finished is harmless and has no effect.
    # 
    # @return [void]
    def stop; end
    
    ##
    # Pauses the sample, to be resumed afterwards.
    # 
    # @note The sample will still occupy a playback channel while paused.
    # 
    # @return [void]
    def pause; end
    
    ##
    # Resumes playback of the sample.
    # 
    # @return [void]
    def resume; end
    
    ##
    # @return [true, false] whether the sample is paused.
    def paused?; end
    
    ##
    # @return [true, false] whether the sample is playing.
    def playing?; end
  end
  
  ##
  # Songs are less flexible than samples in that only one can be played at a time, with no panning or speed control.
  # 
  # @see Gosu::Sample
  class Song
    class <<Song
      ##
      # Returns the song currently being played (even if it's paused), or nil if no song is playing.
      # 
      # @return [Gosu::Song?] the currently playing song.
      attr_reader :current_song
    end
  
    ##
    # @return [Float] the song's playback volume.
    attr_accessor :volume
    
    ##
    # Loads a song from a file.
    # 
    # (Passing a Window reference is not necessary anymore, please use the first overload instead.)
    # 
    # @overload initialize(filename)
    # @overload initialize(window, filename)
    #
    # @param filename [String] the path to load the song from.
    def initialize(window, filename); end
    
    ##
    # Starts or resumes playback of the song.
    # 
    # If another song is currently playing, it will be stopped and this song will be set as the current song.
    # 
    # If `looping` is false, the current song will be set to `nil` when this song finishes.
    # 
    # @return [void]
    # @param looping [true, false] whether the song should play in a loop.
    def play(looping=false); end
    
    ##
    # Pauses playback of the song. The current song is unchanged.
    # 
    # @return [void]
    def pause; end
    
    # Returns true if this song is the current song and playback is paused.
    # 
    # @return [true, false] whether the song is paused.
    def paused?; end
    
    ##
    # Stops playback if this song is the current song. The current song is set to `nil`.
    # 
    # @return [void]
    def stop; end
    
    ##
    # @return [true, false] whether the song is playing.
    def playing?; end
  end
  
  ##
  # A TextInput is an invisible object that handles input using the operating system's input manager.
  #
  # At its most basic, you only need to set {Gosu::Window#text_input} to an instance of this class. The TextInput will then handle all keyboard input until {Gosu::Window#text_input} is set to `nil`. Any text the user has typed is available through {#text}.
  #
  # This class is purely back-end and does not come with a GUI; drawing the input field is up to you, the programmer. The best way to do that is left completely open. TextInput only aims to provide a foundation for you to build your own GUI.
  # 
  # @see Gosu::Window#text_input
  # @see file:examples/TextInput.rb
  class TextInput
    ##
    # @return [String] the text that the user has typed.
    attr_accessor :text
    
    ##
    # @return [Fixnum] the position of the editing caret.
    attr_accessor :caret_pos
    
    ##
    # @return [Fixnum] the starting position of the currently selected text.
    attr_accessor :selection_start
    
    ##
    # This method is an overridable filter that is applied to all newly entered text. This allows for restricting input characters or format, automatic macro or abbreviation expansion and so on.
    # 
    # The return value of this method will be inserted at the current caret position.
    # 
    # The default implementation returns its argument unchanged.
    # 
    # @return [String] the string to be inserted.
    # @param text_in [String] the text typed by the user.
    # 
    # @example Forcing input to all uppercase, alphanumeric characters.
    #   input = TextInput.new
    #   def input.filter(text_in)
    #     text_in.upcase.gsub(/[^A-Z0-9]/, '')
    #   end
    def filter text_in
      text_in
    end
  end
  
  ##
  # Main class that serves as the foundation of a standard
  # Gosu application. Manages initialization of all of Gosu's core components
  # and provides timing functionality.
  #
  # Note that all coordinates, even the mouse position, are in client
  # coordinates relative to the window. This means that the mouse position
  # can be negative or larger than the window size.
  #
  # @note There should really only be one instance of this class at a time. This may or may not change later, but for right now, having two or more windows and loading samples or songs on both of them will result in an exception.
  class Window
    ##
    # @return [String] the window's caption, usually dispalyed in the title bar.
    attr_accessor :caption
    
    ##
    # @return [Fixnum] the mouse pointer's window-based X coordinate.
    attr_accessor :mouse_x
    
    ##
    # @return [Fixnum] the mouse pointer's window-based Y coordinate.
    attr_accessor :mouse_y
    
    ##
    # The currently active {TextInput}. If not nil, all keyboard input will be handled by this object.
    # 
    # @return [TextInput?] the currently active text input.
    attr_accessor :text_input
    
    ##
    # The window's width, in pixels. This only counts the drawable area and does not include any borders or decorations added by the window manager.
    # 
    # @return [Fixnum] the window's width, in pixels.
    attr_reader :width
    
    ##
    # The window's height, in pixels. This only counts the drawable area and does not include any borders or decorations added by the window manager.
    # 
    # @return [Fixnum] the window's height, in pixels.
    attr_reader :height
    
    ##
    # @return [true, false] whether this is a full-screen window.
    attr_reader :fullscreen?
    
    ##
    # @return [Float] the interval between calls to {#update}, in milliseconds.
    attr_reader :update_interval
    
    ##
    # Creates a new window with the requested size.
    # 
    # @note The actual window may be smaller if the requested size is too large for the current display resolution; in that case, Gosu will automatically scale all coordinates to transparently emulate a larger window. No need to thank us.
    # 
    # @overload initialize(width, height, options = {})
    # @overload initialize(width, height, fullscreen, update_interval = 16.666666)
    # 
    # @param width [Fixnum] the desired window width.
    # @param height [Fixnum] the desired window height.
    # @param [Hash] options
    # @option options [true, false] :fullscreen (false) whether to present the window in fullscreen mode.
    # @option options [Float] :update_interval (16.666666) the interval between frames, in milliseconds.
    def initialize(width, height, options); end
    
    ##
    # Enters a modal loop where the Window is visible on screen and receives calls to draw, update etc.
    # 
    # @return [void]
    def show; end
    
    ##
    # Tells the window to end the current run loop as soon as possible. Calling this method will not prevent execution of lines after it.
    # 
    # @return [void]
    def close; end 
    
    # @!group Callbacks
    
    ##
    # This method is called once every {#update_interval} milliseconds while the window is being shown. Your application's main logic should go here.
    # 
    # @return [void]
    def update; end
    
    ##
    # This method is called after every update and whenever the OS wants the window to repaint itself. Your application's rendering code should go here.
    # 
    # @return [void]
    # 
    # @see #needs_redraw?
    def draw; end
    
    ##
    # This method can be overriden to give the game a chance to opt out of a call to {#draw}; however, the operating system can still force a redraw for any reason.
    #
    # @return [true, false] whether the window needs to be redrawn.
    # 
    # @see #draw
    def needs_redraw?; end
    
    ##
    # This method can be overriden to control the visibility of the system cursor over your window, e.g., for level editors or other situations where introducing a custom cursor or hiding the default one is not desired.
    # 
    # @return [true, false] whether the system cursor should be shown.
    def needs_cursor?; end
    
    ##
    # This method is called before {#update} if a button was pressed while the window had focus.
    # 
    # @return [void]
    # @param id [Fixnum] the button's platform-defined id.
    # 
    # @see #button_up
    # @see Gosu.button_down?
    def button_down(id); end
    
    ##
    # This method is called before {#update} if a button was released while the window had focus.
    # 
    # @return [void]
    # @param (see #button_down)
    # 
    # @see #button_down
    # @see Gosu.button_down?
    def button_up(id); end
    
    # @!endgroup
    
    ##
    # @deprecated Use {#mouse_x=} and {#mouse_y=} instead.
    def set_mouse_position(x, y); end
  end
  
  ##
  # Holds information about the underlying OpenGL texture and UV coordinates of an image.
  #
  # Can be retrieved from some images to use them in OpenGL operations.
  #
  # @see Gosu::Image#gl_tex_info
  # @see file:examples/OpenGLIntegration.rb
  class GLTexInfo
    ##
    # @return [Fixnum] OpenGL texture id
    attr_reader :tex_name
    
    ##
    # @return [Float] the U coordinate of the left edge of the image.
    attr_reader :left
    
    ##
    # @return [Float] the U coordinate of the right edge of the image.
    attr_reader :right
    
    ##
    # @return [Float] the V coordinate of the top edge of the image.
    attr_reader :top
    
    ##
    # @return [Float] the V coordinate of the bottom edge of the image.
    attr_reader :bottom
  end

  class << self
    ##
    # Returns whether the button `id` is currently pressed. Button states are updated once per tick, so repeated calls during the same tick will always yield the same result.
    # 
    # @return [true, false] whether the button is currently pressed.
    # @param (see #button_down)
    # 
    # @see Window#button_down
    # @see Window#button_up
    def button_down?(id); end
    
    # @!group Drawing primitives
    
    ##
    # Draws a line from one point to another---inconsistently.
    # 
    # @note OpenGL lines are not reliable at all and may have a missing pixel at the start or end point. Relying on your machine's behavior can only end in tears. Recommended for debugging purposes only.
    # 
    # @return [void]
    # @param x1 [Float] the X coordinate of the start point.
    # @param y1 [Float] the Y coordinate of the start point.
    # @param c1 [Gosu::Color] the color of the start point.
    # @param x2 [Float] the X coordinate of the end point.
    # @param y2 [Float] the Y coordinate of the end point.
    # @param c2 [Gosu::Color] the color of the end point.
    # @param z [Float] the Z-order.
    # @param mode [:default, :additive] the blending mode to use.
    # 
    # @see draw_triangle
    # @see draw_quad
    # @see https://github.com/jlnr/gosu/wiki/Basic-Concepts#drawing-with-colours Drawing with colors, explained in the Gosu Wiki
    # @see https://github.com/jlnr/gosu/wiki/Basic-Concepts#z-ordering Z-ordering explained in the Gosu Wiki
    def draw_line(x1, y1, c1, x2, y2, c2, z=0, mode=:default); end
    
    ##
    # Draws a triangle.
    # 
    # @return [void]
    # @param x1 [Float] the X coordinate of the first vertex.
    # @param y1 [Float] the Y coordinate of the first vertex.
    # @param c1 [Gosu::Color] the color of the first vertex.
    # @param x2 [Float] the X coordinate of the second vertex.
    # @param y2 [Float] the Y coordinate of the second vertex.
    # @param c2 [Gosu::Color] the color of the second vertex.
    # @param x3 [Float] the X coordinate of the third vertex.
    # @param y3 [Float] the Y coordinate of the third vertex.
    # @param c3 [Gosu::Color] the color of the third vertex.
    # @param z [Float] the Z-order.
    # @param mode [:default, :additive] the blending mode to use.
    #
    # @see draw_line
    # @see draw_quad
    # @see https://github.com/jlnr/gosu/wiki/Basic-Concepts#drawing-with-colours Drawing with colors, explained in the Gosu Wiki
    # @see https://github.com/jlnr/gosu/wiki/Basic-Concepts#z-ordering Z-ordering explained in the Gosu Wiki
    def draw_triangle(x1, y1, c1, x2, y2, c2, x3, y3, c3, z=0, mode=:default); end
    
    ##
    # Draws a quad (actually two triangles).
    # 
    # @return [void]
    # @param x1 [Float] the X coordinate of the first vertex.
    # @param y1 [Float] the Y coordinate of the first vertex.
    # @param c1 [Gosu::Color] the color of the first vertex.
    # @param x2 [Float] the X coordinate of the second vertex.
    # @param y2 [Float] the Y coordinate of the second vertex.
    # @param c2 [Gosu::Color] the color of the second vertex.
    # @param x3 [Float] the X coordinate of the third vertex.
    # @param y3 [Float] the Y coordinate of the third vertex.
    # @param c3 [Gosu::Color] the color of the third vertex.
    # @param x4 [Float] the X coordinate of the fourth vertex.
    # @param y4 [Float] the Y coordinate of the fourth vertex.
    # @param c4 [Gosu::Color] the color of the fourth vertex.
    # @param z [Float] the Z-order.
    # @param mode [:default, :additive] the blending mode to use.
    #
    # @see draw_line
    # @see draw_triangle
    # @see https://github.com/jlnr/gosu/wiki/Basic-Concepts#drawing-with-colours Drawing with colors, explained in the Gosu Wiki
    # @see https://github.com/jlnr/gosu/wiki/Basic-Concepts#order-of-corners The order of corners explained in the Gosu Wiki
    # @see https://github.com/jlnr/gosu/wiki/Basic-Concepts#z-ordering Z-ordering explained in the Gosu Wiki
    def draw_quad(x1, y1, c1, x2, y2, c2, x3, y3, c3, x4, y4, c4, z=0, mode=:default); end
    
    # @!endgroup
    # @!group Manipulating the current drawing context
    
    ##
    # Flushes all drawing operations to OpenGL so that Z-ordering can start anew. This is useful for drawing multiple layers that may not have knowledge of each other's Z-ordering, e.g., drawing a HUD on top of the game world or ensuring that a custom cursor is always drawn above everything else.
    # 
    # @return [void]
    def flush; end
    
    ##
    # Runs the block in a clean OpenGL environment.
    # 
    # If a Z-order is given, the block will be scheduled to run between Gosu drawing operations as normal; otherwise, all prior drawing operations will be flushed and the block will be executed immediately.
    # 
    # @note Gosu does not provide access to the underlying OpenGL APIs. A gem like ruby-opengl is required to use custom OpenGL drawing code.
    #
    # @note Gosu rendering functions MUST NOT be used within the block, and {gl} MUST be used only within the call tree of {Window#draw}.
    # 
    # @return [void]
    # @param z [Float] the Z-order.
    # @yield OpenGL code.
    # 
    # @see Window#draw
    # @see file:reference/Z-Ordering
    # @see file:examples/OpenGLIntegration.rb
    def gl(z=nil); end
    
    ##
    # Masks the drawing area inside the block.
    # 
    # @return [void]
    # @param x [Float] the X coordinate of the top left corner,.
    # @param y [Float] the Y coordinate of the top left corner.
    # @param w [Float] the width of the clipping area.
    # @param h [Float] the height of the clipping area.
    # @yield rendering code.
    # 
    # @see Window#draw
    def clip_to(x, y, w, h); end
    
    ##
    # Records all drawing operatons inside the block as a reusable "image". This method can be used to speed rendering of multiple static images, e.g., a fixed tile map.
    # 
    # @note Because the returned object is not a true image---it's implemented using vertex buffers and is not backed by a texture---there are restrictions on how it can be used.
    #
    # @note The width and height of the returned object will be the same values you passed to {record}, regardless of the area you draw on. It is important to pass accurate values if you plan on using {Gosu::Image#draw_as_quad} or {Gosu::Image#draw_rot} with the result later.
    #
    # @return [Gosu::Image] the recorded drawing operations.
    # @param width [Float] the width of the recorded image.
    # @param height [Float] the height of the recorded image.
    # @yield rendering code.
    # 
    # @see Window#draw
    # @see Gosu::Image
    def record(width, height); end
    
    ##
    # Rotates all drawing operatons inside the block.
    # 
    # @return [void]
    # @param angle [Float] the rotation angle.
    # @param around_x [Float] the X coorinate of the rotation origin.
    # @param around_y [Float] the Y coordinate of the rotation origin.
    # @yield rendering code.
    # 
    # @see Window#draw
    # @see scale
    # @see translate
    # @see transform
    def rotate(angle, around_x=0, around_y=0); end
    
    ##
    # Scales all drawing operations inside the block.
    # 
    # @overload scale(scale_x, scale_y = scale_x) { ... }
    # @overload scale(scale_x, scale_y, around_x, around_y) { ... }
    # 
    # @return [void]
    # @param scale_x [Float] the horizontal scaling factor.
    # @param scale_y [Float] the vertical scaling factor.
    # @param around_x [Float] the X coordinate of the scaling origin.
    # @param around_y [Float] the Y coordinate of the scaling origin.
    # @yield rendering code.
    # 
    # @see Window#draw
    # @see rotate
    # @see translate
    # @see transform
    def scale(scale_x, scale_y, around_x, around_y); end
    
    ##
    # Offsets all drawing operations inside the block.
    # 
    # @return [void]
    # @param x [Float] the X offset.
    # @param y [Float] the Y offset.
    # @yield rendering code.
    # 
    # @see Window#draw
    # @see rotate
    # @see scale
    # @see transform
    def translate(x, y); end
    
    ##
    # Applies a free-form matrix transformation to everything drawn in the block.
    # 
    # @return [void]
    # @param m0 [Float]
    # @param m1 [Float]
    # @param m2 [Float]
    # @param m3 [Float]
    # @param m4 [Float]
    # @param m5 [Float]
    # @param m6 [Float]
    # @param m7 [Float]
    # @param m8 [Float]
    # @param m9 [Float]
    # @param m10 [Float]
    # @param m11 [Float]
    # @param m12 [Float]
    # @param m13 [Float]
    # @param m14 [Float]
    # @param m15 [Float]
    # @yield rendering code.
    # 
    # @see Window#draw
    # @see rotate
    # @see scale
    # @see translate
    def transform(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15); end
    
    # @!endgroup
    
    ##
    # Returns the character a button usually produces, if any.
    # 
    # @note For real text input, look at {TextInput} instead.
    # 
    # @return [String?] the character the button usually produces.
    # @param id [Fixnum] the button's platform-defined id.
    # 
    # @see char_to_button_id
    # @see Window#text_input
    # @see TextInput
    def self.button_id_to_char(id); end
    
    ##
    # Returns the button that usually produces a character, if any.
    # 
    # @return [Fixnum?] the button that usually produces the character.
    # @param char [String] the character to query.
    # 
    # @see button_id_to_char
    # @see Window#text_input
    # @see TextInput
    def self.char_to_button_id(char); end
    
    ##
    # @return [Float] a random number in the range (min...max).
    # @param min [Float] the minimum value, inclusive.
    # @param max [Float] the maximum value, exclusive.
    def random(min, max); end

    ##
    # @return [Float] the X component of a vector of angle theta and magnitude r, or the horizontal distance covered by moving r pixels in the direction given by theta.
    # @param theta [Float]
    # @param r [Float]
    def offset_x(theta, r); end

    ##
    # @return [Float] the Y component of a vector of angle theta and magnitude r, or the vertical distance covered by moving r pixels in the direction given by theta.
    # @param theta [Float]
    # @param r [Float]
    def offset_y(theta, r); end

    ##
    # @return [Float] the angular distance from (x1, y1) to (x1, y2) in degrees, where 0.0 is up. Returns 0 if both points are equal.
    # @param x1 [Float]
    # @param y1 [Float]
    # @param x2 [Float]
    # @param y2 [Float]
    def angle(x1, y1, x2, y2); end

    ##
    # @return [Float] the shortest angular distance from angle1 to angle2. This can be negative if counter-clockwise rotation would yield a shorter distance.
    # @param angle1 [Float]
    # @param angle2 [Float]
    def angle_diff(angle1, angle2); end

    ##
    # @return [Float] the distance from (x1, y1) to (x2, y2).
    # @param x1 [Float]
    # @param y1 [Float]
    # @param x2 [Float]
    # @param y2 [Float]
    def distance(x1, y1, x2, y2); end

    ##
    # @note For long-running games, this counter will eventually wrap around to 0 again.
    # 
    # @return [Fixnum] the number of milliseconds elapsed.
    def milliseconds(); end

    ##
    # @return [Fixnum] the current framerate, in frames per second.
    def fps(); end

    # @return [String] the name of a neutral font that is available on the current platform.
    # 
    # @see Gosu::Font
    # @see Gosu::Image#from_text
    def default_font_name(); end

    ##
    # @return [Fixnum] the width (in pixels) of the user's primary screen.
    def screen_width(); end

    # @return [Fixnum] the height (in pixels) of the user's primary screen.
    def screen_height(); end

    ##
    # @return [Fixnum] the maximum width (in 'points') that is available for a non-fullscreen Window.
    # All windows larger than this size will automatically be shrunk to fit.
    def available_width(); end

    # @return [Fixnum] the maximum height (in 'points') that is available for a non-fullscreen Window.
    # All windows larger than this size will automatically be shrunk to fit.
    def available_height(); end

    ##
    # Returns the language code for the user's preferred language.
    # Expect return values such as 'en_US', 'de_DE.UTF-8', 'ja', 'zh-Hans', etc.
    # You can rely only the first two letters being a language abbreviation.
    # 
    # @return [String] the user's preferred language.
    def language(); end
  end
end

##
# Small additions to Numeric to make it easier to integrate Gosu with
# libraries that use radians, like Chipmunk.
class ::Numeric
  ##
  # Converts radians to a Gosu-compatible angle using the formula <tt>self * 180.0 / Math::PI + 90</tt>.
  # 
  # @return [Float] degrees.
  def radians_to_gosu(); end

  ##
  # Converts a Gosu-compatible angle to radians using the formula <tt>(self - 90) * Math::PI / 180.0</tt>.
  # 
  # @return [Float] radians.
  def gosu_to_radians(); end
  
  ##
  # Converts degrees to radians.
  # 
  # @return [Float] radians.
  def degrees_to_radians(); end
  
  ##
  # Converts radians to degrees.
  # 
  # @return [Float] degrees.
  def radians_to_degrees(); end
end
