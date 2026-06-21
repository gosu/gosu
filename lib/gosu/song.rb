module Gosu
  class Song
    def self.current_song
      ptr = GosuFFI.Gosu_Song_current_song()
      GosuFFI.check_last_error
      unless ptr.null?
        @@current_song
      else
        nil
      end
    end

    def initialize(filename)
      __song = GosuFFI.Gosu_Song_create(filename)
      GosuFFI.check_last_error
      @memory_pointer = FFI::AutoPointer.new(__song, GosuFFI.method(:Gosu_Song_destroy))
    end

    def __pointer
      @memory_pointer
    end

    def play(looping = false)
      @@current_song = self
      GosuFFI.Gosu_Song_play(__pointer, looping)
      GosuFFI.check_last_error
    end

    def playing?
      GosuFFI.check_last_error(GosuFFI.Gosu_Song_playing(__pointer))
    end

    def pause
      GosuFFI.Gosu_Song_pause(__pointer)
      GosuFFI.check_last_error
    end

    def paused?
      GosuFFI.check_last_error(GosuFFI.Gosu_Song_paused(__pointer))
    end

    def stop
      GosuFFI.Gosu_Song_stop(__pointer)
      GosuFFI.check_last_error
    end

    def volume
      GosuFFI.check_last_error(GosuFFI.Gosu_Song_volume(__pointer))
    end

    def volume=(double)
      GosuFFI.Gosu_Song_set_volume(__pointer, double)
      GosuFFI.check_last_error
    end
  end
end
