# Encoding: UTF-8
require_relative "test_helper"

# All sound files are from http://www.bigsoundbank.com/
# "These files are free and completely royalty free for all uses."
class TestAudio < Minitest::Test
  include TestHelper
  include InteractiveTests
  
  def test_sample
    skip_on_appveyor

    sound = Gosu::Sample.new(media_path("0614.ogg"))
    channel = sound.play(1, 1, true)
    interactive_cli("Do you hear a Star Wars-like Blaster sound?")
    
    channel.volume = 0.5
    interactive_cli("Is the volume lower than before?")
    channel.volume = 1.0
    
    channel.speed = 2.0
    interactive_cli("Does it play faster and with a higher pitch now?")
    
    channel.pan = -1.0
    interactive_cli("Only left speaker?")
    
    channel.pan =  1.0
    interactive_cli("Only right speaker?")
    
    channel.pan =  0.0
    interactive_cli("Both speakers again?")
    
    refute channel.paused?
    assert channel.playing?
    
    channel.pause
    interactive_cli("Did it stop?")
    
    assert channel.paused?
    refute channel.playing?

    channel.resume
    interactive_cli("Back again?")
    
    refute channel.paused?
    assert channel.playing?    
    
    channel.stop
    
    refute channel.paused?
    refute channel.playing?
    
    channel = sound.play_pan(1, 1, 0.5, false)
    interactive_cli("Right speaker again, but slowed down this time?")
    
  ensure
    channel && channel.stop
  end
  
  class SongTestWindow < Gosu::Window
    def initialize
      super(50, 50)
      self.caption = "Song-Test"
    end
  end
  
  def test_song
    skip_on_appveyor

    win = SongTestWindow.new
    
    assert_nil Gosu::Song.current_song
    
    song = Gosu::Song.new(media_path("0830.ogg"))

    song.play(true)
    interactive_cli("Do you hear a churchbell?") do
      200.times { win.tick }
    end
    
    song.volume = 0.5
    interactive_cli("Is the volume lowered?") do
      200.times { win.tick }
    end
    song.volume = 1.0
    
    assert_equal song, Gosu::Song.current_song
    
    refute song.paused?
    assert song.playing?
    
    song.pause
    interactive_cli("Did it stop?") do
      200.times { win.tick }
    end
    
    assert_equal song, Gosu::Song.current_song
    
    assert song.paused?
    refute song.playing?

    song.play
    interactive_cli("Back again?") do
      200.times { win.tick }
    end
    
    refute song.paused?
    assert song.playing?    
    
    song.stop
    
    refute song.paused?
    refute song.playing?    
    
    assert_nil Gosu::Song.current_song
  end
end
