# Encoding: UTF-8
require_relative "test_helper"

# All sound files are from http://www.bigsoundbank.com/
# "These files are free and completely royalty free for all uses."
class TestAudio < Minitest::Test
  include TestHelper
  include InteractiveTests
  
  def test_sample
    skip_on_appveyor

    sound = Gosu::Sound.new(media_path("0614.ogg"))
    channel = sound.play(volume: 1, speed: 1, looping: true)
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
    
    channel = sound.play_pan(1, volume: 1, speed: 0.5, looping: false)
    interactive_cli("Right speaker again, but slowed down this time?")
    
  ensure
    channel && channel.stop
  end

  def test_song
    skip_on_appveyor

    # assert_nil Gosu::Sound.current_songs
    
    song = Gosu::Sound.new(media_path("0830.ogg"), streaming: true)

    channel = song.play(looping: true)
    interactive_cli("Do you hear a churchbell?") do
      200.times { Gosu::Sound.update }
    end
    
    channel.volume = 0.5
    interactive_cli("Is the volume lowered?") do
      200.times { Gosu::Sound.update }
    end
    channel.volume = 1.0
    

    refute channel.paused?
    assert channel.playing?
    
    channel.pause
    interactive_cli("Did it stop?") do
      200.times { Gosu::Sound.update }
    end
    
    assert channel.paused?
    refute channel.playing?

    channel.resume
    interactive_cli("Back again?") do
      200.times { Gosu::Sound.update }
    end
    
    refute channel.paused?
    assert channel.playing?
    
    channel.stop
    
    refute channel.paused?
    refute channel.playing?
  end
end
