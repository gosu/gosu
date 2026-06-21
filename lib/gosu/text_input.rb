module Gosu
  class TextInput
    def initialize
      __text_input = GosuFFI.Gosu_TextInput_create()
      GosuFFI.check_last_error
      @memory_pointer = FFI::AutoPointer.new(__text_input, GosuFFI.method(:Gosu_TextInput_destroy))

      @__filter_proc = proc { |data, text| protected_filter(text) }
      GosuFFI.Gosu_TextInput_set_filter(__pointer, @__filter_proc, nil)
      GosuFFI.check_last_error
    end

    def __pointer
      @memory_pointer
    end

    def text
      GosuFFI.check_last_error(GosuFFI.Gosu_TextInput_text(__pointer))
    end

    def text=(string)
      GosuFFI.Gosu_TextInput_set_text(__pointer, string.to_s)
      GosuFFI.check_last_error
    end

    def caret_pos
      GosuFFI.check_last_error(GosuFFI.Gosu_TextInput_caret_pos(__pointer))
    end

    def caret_pos=(int)
      GosuFFI.Gosu_TextInput_set_caret_pos(__pointer, int)
      GosuFFI.check_last_error
    end

    def selection_start
      GosuFFI.check_last_error(GosuFFI.Gosu_TextInput_selection_start(__pointer))
    end

    def selection_start=(int)
      GosuFFI.Gosu_TextInput_set_selection_start(__pointer, int)
      GosuFFI.check_last_error
    end

    def filter(text)
      return text
    end

    def insert_text(text)
      GosuFFI.Gosu_TextInput_insert_text(__pointer, text)
      GosuFFI.check_last_error
    end

    def delete_backward
      GosuFFI.Gosu_TextInput_delete_backward(__pointer)
      GosuFFI.check_last_error
    end

    def delete_forward
      GosuFFI.Gosu_TextInput_delete_forward(__pointer)
      GosuFFI.check_last_error
    end

    # Ensures that filter_result is set on C side before filter callback returns
    private def protected_filter(text)
      string = filter(text)
      GosuFFI.Gosu_TextInput_set_filter_result(__pointer, string)
    end
  end
end
