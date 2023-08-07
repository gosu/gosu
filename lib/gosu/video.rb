module Gosu
  class Video
    extend FFI::Library
    ffi_lib Gosu::LIBRARY_PATH

    attach_function :Gosu_Video_create,  [:string, :uint32], :pointer
    attach_function :Gosu_Video_create_scaled,  [:string, :int, :int, :uint32], :pointer
    # attach_function :Gosu_Video_destroy, [:pointer], :void

    attach_function :Gosu_Video_width,  [:pointer], :int
    attach_function :Gosu_Video_height, [:pointer], :int

    attach_function :Gosu_Video_draw,         [:pointer, :double, :double, :double, :double, :double, :uint32, :uint32], :void
    attach_function :Gosu_Video_draw_rot,     [:pointer, :double, :double, :double, :double, :double, :double,
                                               :double, :double, :uint32, :uint32],                                      :void
    attach_function :Gosu_Video_draw_as_quad, [:pointer, :double, :double, :uint32, :double, :double, :uint32,
                                               :double, :double, :uint32, :double, :double, :uint32, :double, :uint32],  :void

    attach_function :Gosu_Video_play,    [:pointer, :bool], :void
    attach_function :Gosu_Video_playing, [:pointer],        :bool
    attach_function :Gosu_Video_pause,   [:pointer],        :void
    attach_function :Gosu_Video_paused,  [:pointer],        :bool
    attach_function :Gosu_Video_stop,    [:pointer],        :void

    attach_function :Gosu_Video_volume,     [:pointer],          :double
    attach_function :Gosu_Video_set_volume, [:pointer, :double], :void
    attach_function :Gosu_Video_length,     [:pointer],          :double
    attach_function :Gosu_Video_position,     [:pointer],          :double
    attach_function :Gosu_Video_set_position, [:pointer, :double], :void

    def initialize(filename, image_flags = 0)
      __video = Gosu_Video_create(filename, image_flags)
      Gosu.check_last_error
      @memory_pointer = FFI::AutoPointer.new(__video, Gosu::Video.method(:release))
    end

    def __pointer
      @memory_pointer
    end

    def width
      Gosu_Video_width(__pointer).tap { Gosu.check_last_error }
    end

    def height
      Gosu_Video_height(__pointer).tap { Gosu.check_last_error }
    end

    def draw(x, y, z, scale_x = 1, scale_y = 1, color = Gosu::Color::WHITE, flags = :default)
      Gosu_Video_draw(__pointer, x, y, z, scale_x, scale_y, Gosu.color_to_drawop(color), Gosu.blend_mode(flags))
      Gosu.check_last_error
    end

    def draw_rot(x, y, z, angle, center_x = 0.5, center_y = 0.5, scale_x = 1, scale_y = 1, color = Gosu::Color::WHITE, flags = :default)
      Gosu_Video_draw_rot(__pointer, x, y, z, angle, center_x, center_y, scale_x, scale_y, Gosu.color_to_drawop(color), Gosu.blend_mode(flags))
      Gosu.check_last_error
    end

    def draw_as_quad(x1, y1, color1, x2, y2, color2, x3, y3, color3, x4, y4, color4, z = 0, mode = :default)
      Gosu_Video_draw_as_quad(x1, y1, Gosu.color_to_drawop(color1), x2, y2, Gosu.color_to_drawop(color2),
                              x3, y3, Gosu.color_to_drawop(color3), x4, y4, Gosu.color_to_drawop(color4),
                              z, Gosu.blend_mode(mode))
      Gosu.check_last_error
    end

    def play(looping = false)
      Gosu_Video_play(__pointer, looping)
      Gosu.check_last_error
    end

    def playing?
      Gosu_Video_playing(__pointer).tap { Gosu.check_last_error }
    end

    def pause
      Gosu_Video_pause(__pointer)
      Gosu.check_last_error
    end

    def paused?
      Gosu_Video_paused(__pointer).tap { Gosu.check_last_error }
    end

    def stop
      Gosu_Video_stop(__pointer)
      Gosu.check_last_error
    end

    def volume
      Gosu_Video_volume(__pointer).tap { Gosu.check_last_error }
    end

    def volume=(double)
      Gosu_Video_set_volume(__pointer, double.clamp(0.0, 1.0))
      Gosu.check_last_error
    end

    def length
      Gosu_Video_length(__pointer).tap { Gosu.check_last_error }
    end

    def position
      Gosu_Video_position(__pointer).tap { Gosu.check_last_error }
    end

    def position=(double)
      Gosu_Video_set_position(__pointer, double.clamp(0.0, 1.0))
      Gosu.check_last_error
    end

    def self.release(pointer)
      Gosu_Video_destroy(pointer)
      Gosu.check_last_error
    end
  end
end