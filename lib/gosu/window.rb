module Gosu
  class Window
    extend FFI::Library
    ffi_lib Gosu::LIBRARY_PATH

    callback :_callback,                               [:pointer],             :void
    callback :_callback_with_unsigned,                 [:pointer, :uint32],    :void
    callback :_callback_with_string,                   [:pointer, :string],    :void
    callback :_callback_returns_bool,                  [:pointer],             :bool
    callback :_callback_hit_test_returns_unsigned,     [:pointer, :int, :int], :uint32

    attach_function :Gosu_Window_create,  [:int, :int, :uint32, :double], :pointer
    attach_function :Gosu_Window_destroy, [:pointer],                     :void

    attach_function :Gosu_Window_set_draw,                 [:pointer, :_callback, :pointer],                           :void
    attach_function :Gosu_Window_set_update,               [:pointer, :_callback, :pointer],                           :void
    attach_function :Gosu_Window_set_button_down,          [:pointer, :_callback_with_unsigned, :pointer],             :void
    attach_function :Gosu_Window_set_button_up,            [:pointer, :_callback_with_unsigned, :pointer],             :void
    attach_function :Gosu_Window_set_gamepad_connected,    [:pointer, :_callback_with_unsigned, :pointer],             :void
    attach_function :Gosu_Window_set_gamepad_disconnected, [:pointer, :_callback_with_unsigned, :pointer],             :void
    attach_function :Gosu_Window_set_drop,                 [:pointer, :_callback_with_string, :pointer],               :void
    attach_function :Gosu_Window_set_needs_redraw,         [:pointer, :_callback_returns_bool, :pointer],              :void
    attach_function :Gosu_Window_set_needs_cursor,         [:pointer, :_callback_returns_bool, :pointer],              :void
    #attach_function :Gosu_Window_set_capture_cursor,       [:pointer, :_callback_returns_bool, :pointer],              :void
    #attach_function :Gosu_Window_set_hit_test,             [:pointer, :_callback_hit_test_returns_unsigned, :pointer], :void
    attach_function :Gosu_Window_set_close,                [:pointer, :_callback, :pointer],                           :void
    attach_function :Gosu_Window_set_gain_focus,           [:pointer, :_callback, :pointer],                           :void
    attach_function :Gosu_Window_set_lose_focus,           [:pointer, :_callback, :pointer],                           :void

    # Enable gosu's default button_down fullscreen toggle
    attach_function :Gosu_Window_default_button_down, [:pointer, :uint32], :void

    attach_function :Gosu_Window_show,                [:pointer],                    :void
    attach_function :Gosu_Window_tick,                [:pointer],                    :bool
    attach_function :Gosu_Window_close_immediately,   [:pointer],                    :void

    attach_function :Gosu_Window_width,               [:pointer],                    :int
    attach_function :Gosu_Window_set_width,           [:pointer, :int],              :void
    attach_function :Gosu_Window_height,              [:pointer],                    :int
    attach_function :Gosu_Window_set_height,          [:pointer, :int],              :void

    attach_function :Gosu_Window_mouse_x,             [:pointer],                    :double
    attach_function :Gosu_Window_set_mouse_x,         [:pointer, :double],           :void
    attach_function :Gosu_Window_mouse_y,             [:pointer],                    :double
    attach_function :Gosu_Window_set_mouse_y,         [:pointer, :double],           :void

    attach_function :Gosu_Window_caption,             [:pointer],                    :string
    attach_function :Gosu_Window_set_caption,         [:pointer, :string],           :void
    attach_function :Gosu_Window_update_interval,     [:pointer],                    :double
    attach_function :Gosu_Window_set_update_interval, [:pointer, :double],           :void
    attach_function :Gosu_Window_resize,              [:pointer, :int, :int, :bool], :void
    attach_function :Gosu_Window_is_fullscreen,       [:pointer],                    :bool
    attach_function :Gosu_Window_is_resizable,        [:pointer],                    :bool
    attach_function :Gosu_Window_is_borderless,       [:pointer],                    :bool
    attach_function :Gosu_Window_set_fullscreen,      [:pointer, :bool],             :void
    attach_function :Gosu_Window_set_resizable,       [:pointer, :bool],             :void
    attach_function :Gosu_Window_set_borderless,      [:pointer, :bool],             :void

    attach_function :Gosu_Window_sdl_window,          [:pointer],                    :pointer

    #attach_function :Gosu_Window_minimize,            [:pointer],                    :void
    #attach_function :Gosu_Window_restore,             [:pointer],                    :void
    #attach_function :Gosu_Window_maximize,            [:pointer],                    :void

    attach_function :Gosu_Window_text_input,          [:pointer],                    :pointer
    attach_function :Gosu_Window_set_text_input,      [:pointer, :pointer],          :void


    def initialize(width, height, _fullscreen = nil, _update_interval = nil, _resizable = nil, _borderless = nil,
                   fullscreen: false, update_interval: 16.66666667, resizable: false, borderless: false)
      fullscreen = _fullscreen if _fullscreen
      update_interval = _update_interval if _update_interval
      resizable = _resizable if _resizable
      borderless = _borderless if _borderless

      window_flags = Gosu.window_flags(fullscreen: fullscreen, resizable: resizable, borderless: borderless)

      __window = Gosu_Window_create(width, height, window_flags, update_interval)
      Gosu.check_last_error
      @memory_pointer = FFI::AutoPointer.new(__window, Gosu::Window.method(:release))
      @text_input = nil

      {
        update: [],
        draw: [],
        button_down: [:id],
        button_up: [:id],
        gamepad_connected: [:id],
        gamepad_disconnected: [:id],
        drop: [:filename],
        needs_redraw?: [],
        needs_cursor?: [],
        # capture_cursor?: [],
        # hit_test: [:x, :y],
        close: [],
        gain_focus: [],
        lose_focus: []
      }.each do |callback, args|
        callback_safename = callback.to_s.sub("?", "")

        instance_eval("@__#{callback_safename}_proc = proc { |#{[:data, args].flatten.join(',')}| protected_#{callback}(#{args.join(',')}) }")
        send(:"Gosu_Window_set_#{callback_safename}", __pointer, instance_variable_get(:"@__#{callback_safename}_proc"), nil)
        Gosu.check_last_error
      end
    end

    # Returns FFI pointer of C side Gosu::Window
    def __pointer
      @memory_pointer
    end

    def protected_draw
      super

      $gosu_gl_blocks.clear
    end

    def update
    end

    def draw
    end

    def button_down(id)
      Gosu_Window_default_button_down(__pointer, id)
      Gosu.check_last_error
    end

    def button_up(id)
    end

    def gamepad_connected(id)
    end

    def gamepad_disconnected(id)
    end

    def drop(filename)
    end

    def needs_redraw?
      true
    end

    def needs_cursor?
      false
    end

    def capture_cursor?
      false
    end

    def hit_test(x, y)
      0
    end

    def close
      close!
    end

    def gain_focus
    end

    def lose_focus
    end

    def caption
      Gosu_Window_caption(__pointer).tap { Gosu.check_last_error }
    end

    def caption=(text)
      Gosu_Window_set_caption(__pointer, text)
      Gosu.check_last_error
    end

    def fullscreen?
      Gosu_Window_is_fullscreen(__pointer).tap { Gosu.check_last_error }
    end

    def fullscreen=(boolean)
      Gosu_Window_resize(__pointer, width, height, !!boolean)
      Gosu.check_last_error
    end

    def resizable?
      Gosu_Window_is_resizable(__pointer).tap { Gosu.check_last_error }
    end

    def resizable=(boolean)
      Gosu_Window_set_resizable(__pointer, !!boolean)
      Gosu.check_last_error
    end

    def borderless?
      Gosu_Window_is_borderless(__pointer).tap { Gosu.check_last_error }
    end

    def borderless=(boolean)
      Gosu_Window_set_borderless(__pointer, !!boolean)
      Gosu.check_last_error
    end

    def text_input
      @text_input || nil
    end

    def text_input=(text_input)
      raise ArgumentError, "text_input must be a Gosu::TextInput" unless text_input.is_a?(Gosu::TextInput) || text_input.nil?

      ptr = text_input ? text_input.__pointer : nil
      @text_input = text_input

      Gosu_Window_set_text_input(__pointer, ptr)
      Gosu.check_last_error
    end

    def update_interval
      Gosu_Window_update_interval(__pointer).tap { Gosu.check_last_error }
    end

    def update_interval=(double)
      Gosu_Window_set_update_interval(__pointer, double)
      Gosu.check_last_error
    end

    def width
      Gosu_Window_width(__pointer).tap { Gosu.check_last_error }
    end

    def width=(int)
      Gosu_Window_resize(__pointer, int, height, fullscreen?)
      Gosu.check_last_error
    end

    def height
      Gosu_Window_height(__pointer).tap { Gosu.check_last_error }
    end

    def height=(int)
      Gosu_Window_resize(__pointer, width, int, fullscreen?)
      Gosu.check_last_error
    end

    def mouse_x
      Gosu_Window_mouse_x(__pointer).tap { Gosu.check_last_error }
    end

    def mouse_x=(double)
      Gosu_Window_set_mouse_x(__pointer, double)
      Gosu.check_last_error
    end

    def mouse_y
      Gosu_Window_mouse_y(__pointer).tap { Gosu.check_last_error }
    end

    def mouse_y=(double)
      Gosu_Window_set_mouse_y(__pointer, double)
      Gosu.check_last_error
    end

    def set_mouse_position(x, y)
      self.mouse_x = x
      self.mouse_y = y
    end

    def show
      Gosu_Window_show(__pointer)

      raise @__exception if defined?(@__exception)

      Gosu.check_last_error
    end

    def tick
      value = Gosu_Window_tick(__pointer).tap { Gosu.check_last_error }

      raise @__exception if defined?(@__exception)

      value
    end

    def minimize
      Gosu_Window_minimize(__pointer)
      Gosu.check_last_error
    end

    def restore
      Gosu_Window_restore(__pointer)
      Gosu.check_last_error
    end

    def maximize
      Gosu_Window_maximize(__pointer)
      Gosu.check_last_error
    end

    def close!
      Gosu_Window_close_immediately(__pointer)
      Gosu.check_last_error
    end

    def sdl_window
      Gosu_Window_sdl_window(__pointer).tap { Gosu.check_last_error }
    end

    def self.release(pointer)
      Gosu_Window_destroy(pointer)
      Gosu.check_last_error
    end
  end
end
