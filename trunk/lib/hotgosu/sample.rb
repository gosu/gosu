module Gosu
  class Sample < ::ObjGosuSample
    def self.new(window, filename)
      alloc.initWithWindow window, filename: filename
    end
    
    def play(vol = 1, speed = 1, looping = false)
      playWithVolume vol, speed: speed, looping: looping
    end
    
    def play_pan(pan = 0, vol = 1, speed = 1, looping = false)
      playWithPan pan, volume: vol, speed: speed, looping: looping
    end
  end
end
