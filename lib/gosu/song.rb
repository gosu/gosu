module Gosu
  class Song
    extend FFI::Library
    ffi_lib Gosu::LIBRARY_PATH

    attach_function :Gosu_Song_current_song, [], :pointer

    attach_function :Gosu_Song_create,  [:string], :pointer
    attach_function :Gosu_Song_destroy, [:pointer], :void

    attach_function :Gosu_Song_play,    [:pointer, :bool], :void
    attach_function :Gosu_Song_playing, [:pointer],        :bool
    attach_function :Gosu_Song_pause,   [:pointer],        :void
    attach_function :Gosu_Song_paused,  [:pointer],        :bool
    attach_function :Gosu_Song_stop,    [:pointer],        :void

    attach_function :Gosu_Song_volume,     [:pointer],          :double
    attach_function :Gosu_Song_set_volume, [:pointer, :double], :void

    def self.current_song
      ptr = Gosu_Song_current_song()
      Gosu.check_last_error
      unless ptr.null?
        @@current_song
      else
        nil
      end
    end

    def initialize(filename)
      __song = Gosu_Song_create(filename)
      Gosu.check_last_error
      @memory_pointer = FFI::AutoPointer.new(__song, Gosu::Song.method(:release))
    end

    def __pointer
      @memory_pointer
    end

    def play(looping = false)
      @@current_song = self
      Gosu_Song_play(__pointer, looping)
      Gosu.check_last_error
    end

    def playing?
      Gosu_Song_playing(__pointer).tap { Gosu.check_last_error }
    end

    def pause
      Gosu_Song_pause(__pointer)
      Gosu.check_last_error
    end

    def paused?
      Gosu_Song_paused(__pointer).tap { Gosu.check_last_error }
    end

    def stop
      Gosu_Song_stop(__pointer)
      Gosu.check_last_error
    end

    def volume
      Gosu_Song_volume(__pointer).tap { Gosu.check_last_error }
    end

    def volume=(double)
      Gosu_Song_set_volume(__pointer, double.clamp(0.0, 1.0))
      Gosu.check_last_error
    end

    def self.release(pointer)
      Gosu_Song_destroy(pointer)
      Gosu.check_last_error
    end
  end
end