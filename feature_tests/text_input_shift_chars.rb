# $LOAD_PATH << '../lib'
require 'gosu'

N = 10

# requires xdotool
class Test < Gosu::Window
    def initialize
        super(400, 100, false)

        self.text_input = Gosu::TextInput.new()
        self.caption    = rand(0xFFFFFFFF).to_s
        @n      = 0
        @pass   = false
    end

    # NOTE: this test doesn't `work` everytime for reasons unrelated to the
    # bug that prompted this test; run the test N times and make sure it
    # passes at least once (and find someone better at stats to determine what
    # N should be)
    def draw
        id = `xdotool search --name #{self.caption}`.split("\n").first
        s = "123!@#abcABC?"
        `xdotool type --window #{id} #{s}`

        if self.text_input.text() == s then
            @pass = true
        end

        if @n > N then
            if false == @pass then
                fail "failure"
            else
                puts "success"
                exit(0)
            end
        end

        @n += 1
        self.text_input = Gosu::TextInput.new()
    end
end

Test.new().show()
