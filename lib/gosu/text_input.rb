module Gosu
  class TextInput
    extend FFI::Library
    ffi_lib Gosu::LIBRARY_PATH

    callback :_callback_with_string, [:pointer, :string], :void

    attach_function :Gosu_TextInput_create,   [],         :pointer
    attach_function :Gosu_TextInput_destroy,  [:pointer], :void

    attach_function :Gosu_TextInput_caret_pos,           [:pointer],          :uint32
    attach_function :Gosu_TextInput_set_caret_pos,       [:pointer, :uint32], :void
    attach_function :Gosu_TextInput_selection_start,     [:pointer],          :uint32
    attach_function :Gosu_TextInput_set_selection_start, [:pointer, :uint32], :void

    attach_function :Gosu_TextInput_text,              [:pointer],                                   :string
    attach_function :Gosu_TextInput_set_text,          [:pointer, :string],                          :void
    attach_function :Gosu_TextInput_set_filter,        [:pointer, :_callback_with_string, :pointer], :void
    attach_function :Gosu_TextInput_set_filter_result, [:pointer, :string],                          :void

    attach_function :Gosu_TextInput_insert_text,     [:pointer, :string], :void
    attach_function :Gosu_TextInput_delete_backward, [:pointer],          :void
    attach_function :Gosu_TextInput_delete_forward,  [:pointer],          :void

    def initialize
      __text_input = Gosu_TextInput_create()
      Gosu.check_last_error
      @memory_pointer = FFI::AutoPointer.new(__text_input, Gosu::TextInput.method(:release))

      @__filter_proc = proc { |data, text| protected_filter(text) }
      Gosu_TextInput_set_filter(__pointer, @__filter_proc, nil)
      Gosu.check_last_error
    end

    def __pointer
      @memory_pointer
    end

    def text
      Gosu_TextInput_text(__pointer).tap { Gosu.check_last_error }
    end

    def text=(string)
      Gosu_TextInput_set_text(__pointer, string.to_s)
      Gosu.check_last_error
    end

    def caret_pos
      Gosu_TextInput_caret_pos(__pointer).tap { Gosu.check_last_error }
    end

    def caret_pos=(int)
      Gosu_TextInput_set_caret_pos(__pointer, int)
      Gosu.check_last_error
    end

    def selection_start
      Gosu_TextInput_selection_start(__pointer).tap { Gosu.check_last_error }
    end

    def selection_start=(int)
      Gosu_TextInput_set_selection_start(__pointer, int)
      Gosu.check_last_error
    end

    def filter(text)
      return text
    end

    def insert_text(text)
      Gosu_TextInput_insert_text(__pointer, text)
      Gosu.check_last_error
    end

    def delete_backward
      Gosu_TextInput_delete_backward(__pointer)
      Gosu.check_last_error
    end

    def delete_forward
      Gosu_TextInput_delete_forward(__pointer)
      Gosu.check_last_error
    end

    # Ensures that filter_result is set on C side before filter callback returns
    private def protected_filter(text)
      string = filter(text)
      Gosu_TextInput_set_filter_result(__pointer, string)
    end

    def self.release(pointer)
      Gosu_TextInput_destroy(pointer)
      Gosu.check_last_error
    end
  end
end