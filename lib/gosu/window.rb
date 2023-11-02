module Gosu
  class Window
    def initialize(width, height, _fullscreen = nil, _update_interval = nil, _resizable = nil, _borderless = nil,
                   fullscreen: false, update_interval: 16.66666667, resizable: false, borderless: false)
      fullscreen = _fullscreen unless _fullscreen.nil?
      update_interval = _update_interval unless _update_interval.nil?
      resizable = _resizable unless _resizable.nil?
      borderless = _borderless unless _borderless.nil?

      window_flags = GosuFFI.window_flags(fullscreen: fullscreen, resizable: resizable, borderless: borderless)

      __window = GosuFFI.Gosu_Window_create(width, height, window_flags, update_interval)
      GosuFFI.check_last_error
      @memory_pointer = FFI::AutoPointer.new(__window, GosuFFI.method(:Gosu_Window_destroy))
      @text_input = nil

      # Set up callbacks without arguments.
      %w(update draw needs_redraw needs_cursor close gain_focus lose_focus).each do |callback|
        instance_variable_set(:"@__#{callback}_proc", proc do |data|
          send(callback)
        rescue Exception => e
          @__exception = e
          GosuFFI.Gosu_Window_close_immediately(__pointer)
          # No need to check errors after this.
          false
        ensure
          $gosu_gl_blocks = nil if callback == "draw"
        end)
        GosuFFI.send(:"Gosu_Window_set_#{callback}", __pointer, instance_variable_get(:"@__#{callback}_proc"), nil)
        GosuFFI.check_last_error
      end

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
        close: [],
        gain_focus: [],
        lose_focus: [],
      }.each do |callback, args|
        callback_safename = callback.to_s.sub("?", "")

        instance_eval("@__#{callback_safename}_proc = proc { |#{[:data, args].flatten.join(",")}| protected_#{callback}(#{args.join(",")}) }")
        GosuFFI.send(:"Gosu_Window_set_#{callback_safename}", __pointer, instance_variable_get(:"@__#{callback_safename}_proc"), nil)
        GosuFFI.check_last_error
      end
    end

    # Returns FFI pointer of C side Gosu::Window
    def __pointer
      @memory_pointer
    end

    def update
    end

    def draw
    end

    def button_down(id)
      GosuFFI.Gosu_Window_default_button_down(__pointer, id)
      GosuFFI.check_last_error
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

    def close
      close!
    end

    def gain_focus
    end

    def lose_focus
    end

    def caption
      GosuFFI.check_last_error(GosuFFI.Gosu_Window_caption(__pointer))
    end

    def caption=(text)
      GosuFFI.Gosu_Window_set_caption(__pointer, text)
      GosuFFI.check_last_error
    end

    def fullscreen?
      GosuFFI.check_last_error(GosuFFI.Gosu_Window_is_fullscreen(__pointer))
    end

    def fullscreen=(boolean)
      GosuFFI.Gosu_Window_resize(__pointer, width, height, !!boolean)
      GosuFFI.check_last_error
    end

    def resizable?
      GosuFFI.check_last_error(GosuFFI.Gosu_Window_is_resizable(__pointer))
    end

    def resizable=(boolean)
      GosuFFI.Gosu_Window_set_resizable(__pointer, !!boolean)
      GosuFFI.check_last_error
    end

    def borderless?
      GosuFFI.check_last_error(GosuFFI.Gosu_Window_is_borderless(__pointer))
    end

    def borderless=(boolean)
      GosuFFI.Gosu_Window_set_borderless(__pointer, !!boolean)
      GosuFFI.check_last_error
    end

    def text_input
      @text_input || nil
    end

    def text_input=(text_input)
      if not text_input.nil? and not text_input.is_a? Gosu::TextInput
        raise ArgumentError, "text_input must be nil or a Gosu::TextInput"
      end

      @text_input = text_input

      GosuFFI.Gosu_Window_set_text_input(__pointer, text_input&.__pointer)
      GosuFFI.check_last_error
    end

    def update_interval
      GosuFFI.check_last_error(GosuFFI.Gosu_Window_update_interval(__pointer))
    end

    def update_interval=(double)
      GosuFFI.Gosu_Window_set_update_interval(__pointer, double)
      GosuFFI.check_last_error
    end

    def width
      GosuFFI.check_last_error(GosuFFI.Gosu_Window_width(__pointer))
    end

    def width=(int)
      GosuFFI.Gosu_Window_resize(__pointer, int, height, fullscreen?)
      GosuFFI.check_last_error
    end

    def height
      GosuFFI.check_last_error(GosuFFI.Gosu_Window_height(__pointer))
    end

    def height=(int)
      GosuFFI.Gosu_Window_resize(__pointer, width, int, fullscreen?)
      GosuFFI.check_last_error
    end

    def mouse_x
      GosuFFI.check_last_error(GosuFFI.Gosu_Window_mouse_x(__pointer))
    end

    def mouse_x=(double)
      GosuFFI.Gosu_Window_set_mouse_x(__pointer, double)
      GosuFFI.check_last_error
    end

    def mouse_y
      GosuFFI.check_last_error(GosuFFI.Gosu_Window_mouse_y(__pointer))
    end

    def mouse_y=(double)
      GosuFFI.Gosu_Window_set_mouse_y(__pointer, double)
      GosuFFI.check_last_error
    end

    def show
      GosuFFI.Gosu_Window_show(__pointer)

      raise @__exception if defined?(@__exception)

      GosuFFI.check_last_error
    end

    def tick
      value = GosuFFI.check_last_error(GosuFFI.Gosu_Window_tick(__pointer))

      raise @__exception if defined?(@__exception)

      value
    end

    def close!
      GosuFFI.Gosu_Window_close_immediately(__pointer)
      GosuFFI.check_last_error
    end

    def sdl_window
      GosuFFI.check_last_error(GosuFFI.Gosu_Window_sdl_window(__pointer))
    end
  end
end
