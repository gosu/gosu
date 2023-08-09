module Gosu
  class Channel
    def initialize(pointer)
      @managed_pointer = FFI::AutoPointer.new(pointer, GosuFFI.method(:Gosu_Channel_destroy))
    end

    def __pointer
      @managed_pointer
    end

    def playing?
      GosuFFI.check_last_error(GosuFFI.Gosu_Channel_playing(__pointer))
    end

    def pause
      GosuFFI.Gosu_Channel_pause(__pointer)
      GosuFFI.check_last_error
    end

    def paused?
      GosuFFI.check_last_error(GosuFFI.Gosu_Channel_paused(__pointer))
    end

    def resume
      GosuFFI.Gosu_Channel_resume(__pointer)
      GosuFFI.check_last_error
    end

    def stop
      GosuFFI.Gosu_Channel_stop(__pointer)
      GosuFFI.check_last_error
    end

    def volume=(double)
      GosuFFI.Gosu_Channel_set_volume(__pointer, double)
      GosuFFI.check_last_error
    end

    def speed=(double)
      GosuFFI.Gosu_Channel_set_speed(__pointer, double)
      GosuFFI.check_last_error
    end

    def pan=(double)
      GosuFFI.Gosu_Channel_set_pan(__pointer, double)
      GosuFFI.check_last_error
    end
  end
end
