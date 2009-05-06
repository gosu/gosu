#!/usr/bin/env ruby
require 'rubygems'

spec = Gem::Specification.new do |s|
  s.name = 'gosu'
  s.version = '0.7.9.2'
  s.authors = ['Julian Raschke', 'Jan Luecker']

  s.description =<<-EOF

    2D game development library.  The library features easy to use and
    game-friendly interfaces to 2D graphics and text (accelerated by
    3D hardware), sound samples and music as well as keyboard, mouse
    and gamepad/joystick input.  Includes demos for integration with
    RMagick, Chipmunk and Ruby-OpenGL.
EOF

  s.email = 'julian@raschke.de'

  s.files = [
             'README.txt',
             'COPYING.txt',
             'examples/ChipmunkIntegration.rb',
             'examples/CptnRuby.rb',
             'examples/MoreChipmunkAndRMagick.rb',
             'examples/OpenGLIntegration.rb',
             'examples/RMagickIntegration.rb',
             'examples/TextInput.rb',
             'examples/Tutorial.rb',
             'examples/Tutorial.cpp',
             'examples/media/Beep.wav',
             'examples/media/CptnRuby Gem.png',
             'examples/media/CptnRuby Map.txt',
             'examples/media/CptnRuby Tileset.png',
             'examples/media/CptnRuby.png',
             'examples/media/Cursor.png',
             'examples/media/Earth.png',
             'examples/media/Explosion.wav',
             'examples/media/LargeStar.png',
             'examples/media/Sky.jpg',
             'examples/media/Smoke.png',
             'examples/media/Soldier.png',
             'examples/media/Space.png',
             'examples/media/Star.png',
             'examples/media/Starfighter.bmp',
             'linux/configure.ac',
             'linux/Makefile.in',
             'linux/extconf.rb'
            ]

  s.files += Dir['Gosu/**/**'] + Dir['GosuImpl/**/**']

  s.files.reject! {|f| f.include?('.svn')}

  s.has_rdoc = false
  s.homepage = 'http://code.google.com/p/gosu'
  s.summary = '2D game development library.'

  s.requirements = [ 'See http://code.google.com/p/gosu/wiki/GettingStartedOnLinux\n)"]
end
