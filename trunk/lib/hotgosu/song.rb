module Gosu
  Song = ::ObjGosuSong

  class Song
    def self.current_song
      currentSong
    end
    
    def self.new(window, filename)
      alloc.initWithWindow window, filename: filename
    end
    
    def play(looping = false)
      playWithLoop looping
    end

    def paused?
      isPaused
    end
    
    def playing?
      isPlaying
    end
  end
end
