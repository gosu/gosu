module Gosu
  class Channel
    extend FFI::Library
    ffi_lib Gosu::LIBRARY_PATH

    attach_function :Gosu_Channel_destroy, [:pointer], :void

    attach_function :Gosu_Channel_playing, [:pointer], :bool
    attach_function :Gosu_Channel_pause,   [:pointer], :void
    attach_function :Gosu_Channel_paused,  [:pointer], :bool
    attach_function :Gosu_Channel_resume,  [:pointer], :void
    attach_function :Gosu_Channel_stop,    [:pointer], :void

    attach_function :Gosu_Channel_set_volume, [:pointer, :double], :void
    attach_function :Gosu_Channel_set_speed,  [:pointer, :double], :void
    attach_function :Gosu_Channel_set_pan,    [:pointer, :double], :void

    def initialize(pointer)
      @managed_pointer = FFI::AutoPointer.new(pointer, Gosu::Channel.method(:release))
    end

    def __pointer
      @managed_pointer
    end

    def playing?
      Gosu_Channel_playing(__pointer).tap { Gosu.check_last_error }
    end

    def pause
      Gosu_Channel_pause(__pointer)
      Gosu.check_last_error
    end

    def paused?
      Gosu_Channel_paused(__pointer).tap { Gosu.check_last_error }
    end

    def resume
      Gosu_Channel_resume(__pointer)
      Gosu.check_last_error
    end

    def stop
      Gosu_Channel_stop(__pointer)
      Gosu.check_last_error
    end

    def volume=(double)
      Gosu_Channel_set_volume(__pointer, double.clamp(0.0, 1.0))
      Gosu.check_last_error
    end

    def speed=(double)
      Gosu_Channel_set_speed(__pointer, double.clamp(0.0, 1.0))
      Gosu.check_last_error
    end

    def pan=(double)
      Gosu_Channel_set_pan(__pointer, double.clamp(-1.0, 1.0))
      Gosu.check_last_error
    end

    def self.release(pointer)
      Gosu_Channel_destroy(pointer)
      Gosu.check_last_error
    end
  end
end
