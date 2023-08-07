module Gosu
  class Sample
    extend FFI::Library
    ffi_lib Gosu::LIBRARY_PATH

    attach_function :Gosu_Sample_create,   [:string],  :pointer
    attach_function :Gosu_Sample_destroy,  [:pointer], :void

    attach_function :Gosu_Sample_play,     [:pointer, :double, :double, :bool],          :pointer
    attach_function :Gosu_Sample_play_pan, [:pointer, :double, :double, :double, :bool], :pointer

    def initialize(filename)
      __sample = Gosu_Sample_create(filename)
      Gosu.check_last_error
      @memory_pointer = FFI::AutoPointer.new(__sample, Gosu::Sample.method(:release))
    end

    def __pointer
      @memory_pointer
    end

    def play(volume = 1, speed = 1, looping = false)
      __channel = Gosu_Sample_play(__pointer, volume, speed, looping)
      Gosu.check_last_error
      Gosu::Channel.new(__channel)
    end

    def play_pan(pan = 0, volume = 1, speed = 1, looping = false)
      __channel = Gosu_Sample_play_pan(__pointer, pan, volume, speed, looping)
      Gosu::Channel.new(__channel)
    end

    def self.release(pointer)
      Gosu_Sample_destroy(pointer)
      Gosu.check_last_error
    end
  end
end