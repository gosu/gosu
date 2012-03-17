# Gosu Zen example based on erisdiscord's comment here:
# https://github.com/jlnr/gosu/pull/120

# Gosu Zen is the (inline) Sinatra of Ruby multimedia programming.
# The interface is still in flux. If you want to tune the interface
# a little further or provide more examples, please fork Gosu and
# send a pull request. Thanks!

require 'rubygems'
require 'gosu/zen'
include Gosu::Zen

window 480, 240, :fullscreen => false

button_down Gosu::KbEscape do
  close
end

update do
  t = Gosu.milliseconds / 1000.0

  @radius = [width, height].min * 0.37
  @angle = t * Math::PI

  a, b =\
    (Math.cos(t) + 0.5) * 0xff,
    (Math.sin(t) + 0.5) * 0xff

  c = (a + b) / 2

  @colors = [
    Gosu::Color.rgb(a, b, 0xff),
    Gosu::Color.rgb(a, 0x00, b),
    Gosu::Color.rgb(0xff, b, a),
    Gosu::Color.rgb(b, a, 0x00),
    Gosu::Color.rgb(b, 0xff, a),
    Gosu::Color.rgb(0x00, a, b) ]

  @background = Gosu::Color.rgb(c, c, c)
end

draw do
  draw_quad\
    0,     0,      @background,
    0,     height, @background,
    width, height, @background,
    width, 0,      @background, 0

  translate width / 2, height / 2 do
    @colors.each.with_index do |color, i|

      angle = @angle + i.to_f / @colors.length * 2.0 * Math::PI
      x = @radius * Math.sin(angle)
      y = @radius * Math.cos(angle)
      w, h = x, y

      translate x, y do
        rotate Gosu.radians_to_degrees(angle) do
          draw_quad\
            -w, +h, color,
            -w, -h, color,
            +w, -h, color,
            +w, +h, color, 0
        end
      end
    end
  end
end
