require 'rubygems'
require 'fileutils'
require 'date'
require 'rubygems/package_task'

COMMON_FILES = FileList[
  '.yardopts',
  'COPYING',
  'README.txt',
]

COMMON_CPP_FILES = COMMON_FILES + FileList[
  'examples/*.cpp',
  'examples/media/*',
  'reference/cpp/**/*',
]

COMMON_RUBY_FILES = COMMON_FILES + FileList[
  'Gosu/**/*', # include headers to make inline-C extensions possible
  'lib/**/*.rb',
  'examples/*.rb',
  'examples/media/*',
  'reference/**/*.rb',
  'reference/**/*.rdoc',
]

GOSU_VERSION = ENV['GOSU_RELEASE_VERSION'] || '0.0.0'

# Sets everything except 'platform' and 'files'.
def apply_gemspec_defaults s
  s.name = 'gosu'
  s.version = GOSU_VERSION.dup
  s.summary = '2D game development library.'
  s.description = <<EOS
  2D game development library.

  Gosu features easy to use and game-friendly interfaces to 2D graphics
  and text (accelerated by 3D hardware), sound samples and music as well as
  keyboard, mouse and gamepad/joystick input.

  Also includes demos for integration with RMagick, Chipmunk and OpenGL.
EOS
  s.author = 'Julian Raschke'
  s.date = Time.now.strftime '%Y-%m-%d'
  s.email = 'julian@raschke.de'
  s.homepage = 'http://www.libgosu.org/'
  s.required_ruby_version = Gem::Requirement.new('>= 1.8.2')
  s.summary = '2D game development library.'
  s.rdoc_options = %w(README.txt COPYING reference/gosu.rb) +
    Dir['reference/*.rdoc'] + %w(--title Gosu --main README.txt)
end

def upload filename
  sh "scp -P 22000 '#{filename}' libgosu.org:/Library/WebServer/Documents/libgosu.org/downloads/"
end

def zip filename, files
  sh "zip #{filename} #{files.map { |fn| "'#{fn}'" }.join(' ')}"
end

def tar filename, files
  sh "COPYFILE_DISABLE=true tar -czf #{filename} #{files.to_a.uniq.map { |fn| "'#{fn}'" }.join(' ')}"
end

Dir['./rake/*.rb'].each { |task| require task }

task :release => [# Broken - these can only be built on a 10.4-10.6 Machine
                  # :'mac:release', :'mac:release_gem', :'mac:app_wrapper',
                  :'win:release', :'win:release_gem',
                  :'linux:release', :'linux:release_gem']
