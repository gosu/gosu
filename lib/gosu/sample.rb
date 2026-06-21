module Gosu
  class Sample
    def initialize(filename)
      __sample = GosuFFI.check_last_error(GosuFFI.Gosu_Sample_create(filename))
      @memory_pointer = FFI::AutoPointer.new(__sample, GosuFFI.method(:Gosu_Sample_destroy))
    end

    def __pointer
      @memory_pointer
    end

    def play(volume = 1, speed = 1, looping = false)
      __channel = GosuFFI.Gosu_Sample_play(__pointer, volume, speed, looping)
      GosuFFI.check_last_error
      Gosu::Channel.new(__channel)
    end

    def play_pan(pan = 0, volume = 1, speed = 1, looping = false)
      __channel = GosuFFI.Gosu_Sample_play_pan(__pointer, pan, volume, speed, looping)
      GosuFFI.check_last_error
      Gosu::Channel.new(__channel)
    end
  end
end
