$LOAD_PATH << '../lib'
require 'gosu'

class Test < Gosu::Window
  def initialize 
    super(640, 480, false)
    @samples = []
    @samples << Gosu::Sample.new(self, "audio_formats/wav_16bit_pcm.wav")
    @samples << Gosu::Sample.new(self, "audio_formats/ogg_vorbis.ogg")
    @instance = @samples.first.play(1, 1, true)
  end

  def update
    self.caption = "Playing: #{@instance.playing?}, paused: #{@instance.paused?}"
    @instance.pan = Gosu::random(-1, 1) if button_down?(Gosu::KbRight)
  end
  
  def button_down(id)
    case id
    when Gosu::KbEscape
      close
    when Gosu::KbSpace
      @instance.stop
    when Gosu::KbLeft
      @instance.speed = 0.5
    when Gosu::KbDown
      @instance.pause
    when Gosu::KbUp
      @instance.resume
    when Gosu::KbTab
      @instance.stop
      @samples << @samples.shift
      @instance = @samples.first.play(1, 1, true)
    end
  end
end

Test.new.show
