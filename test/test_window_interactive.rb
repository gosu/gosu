# Encoding: UTF-8

if ENV["GOSU_TEST_INTERACTIVE"]
  require "minitest/autorun"
  require "gosu" unless defined? Gosu

  class TestWindowInteractive < Minitest::Test
    class DropWindow < Gosu::Window
      def initialize
        super(800,600)
        self.caption = "(Drag &) Drop Test"

        @font = Gosu::Font.new(15)
        @files = ["Please drop any number of files and make sure all names appear!"]
        @files = ["Press Y if everything is ok or N if something went wrong!"]
      end

      def drop(path)
        @files << path
      end

      def draw
        @files.each_with_index do |file,idx|
          @font.draw file, 10, idx * 20, 0
        end
      end

      def button_down(id)
        if id == Gosu::KB_Y
          puts "Drop-Test passed!"
          close
        end
        if id == Gosu::KB_N
          puts "Drop-Test failed!"
          close
        end
      end
    end

    def open_file_or_folder(path)
      if RUBY_PLATFORM =~ /darwin[0-9]*$/
        `open '#{path}'`
      elsif RUBY_PLATFORM =~ /mingw[0-9]*$/
        `explorer "#{path.gsub('/', '\\')}"`
      else
        `xdg-open '#{path}'`
      end
    end

    def test_drop_callback
      assert_output "Drop-Test passed!", "" do
        open_file_or_folder(__dir__)
        DropWindow.new.show
      end
    end
  end
end
