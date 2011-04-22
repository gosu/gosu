require 'rubygems'
require 'fileutils'
require 'date'
require 'rake/gempackagetask'

COMMON_FILES = FileList[
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
  'lib/gosu.rb',
  'lib/gosu/patches.rb',
  'lib/gosu/swig_patches.rb',
  'examples/*.rb',
  'examples/media/*',
]

GOSU_VERSION = ENV['GOSU_RELEASE_VERSION'] || '0.0.0'

# Sets everything except 'platform' and 'files'.
def apply_gemspec_defaults s
  s.name = 'gosu'
  s.version = GOSU_VERSION
  s.summary = '2D game development library.'
  s.description = <<EOS
  2D game development library.

  Gosu features easy to use and game-friendly interfaces to 2D graphics
  and text (accelerated by 3D hardware), sound samples and music as well as
  keyboard, mouse and gamepad/joystick input.

  Also includes demos for integration with RMagick, Chipmunk and Ruby-OpenGL.
EOS
  s.authors = ['Julian Raschke', 'Jan Luecker']
  s.date = Time.now.strftime '%Y-%m-%d'
  s.email = 'julian@raschke.de'
  s.homepage = 'http://libgosu.org/'
  s.require_paths = ['lib']
  s.required_ruby_version = Gem::Requirement.new('>= 1.8.2')
  s.summary = '2D game development library.'
end

# Broken :( - using scp for now.
# file 'rake/upload' do
#   sh 'curl https://github.com/jlnr/github-upload/raw/master/upload.rb > rake/upload'
#   sh 'chmod +x rake/upload'
# end

def upload filename
  sh "scp -P 22000 '#{filename}' libgosu.org:/Library/WebServer/Documents/libgosu.org/downloads/"
end

def zip filename, files
  sh "zip #{filename} #{files.map { |fn| "'#{fn}'" }.join(' ')}"
  # Apparently not necessary - no weird resource forks in there? Nice!
  # sh "zip -d #{filename} '__MACOSX*' '*.DS_Store'" if `uname`.chomp == 'Darwin'
end

def tar filename, files
  sh "COPYFILE_DISABLE=true tar -czf #{filename} #{files.map { |fn| "'#{fn}'" }.join(' ')}"
end

Dir['rake/*.rb'].each { |task| require task }

task :release => [:'mac:release', :'win:release', :'linux:release',
                  :'mac:release_gem', :'win:release_gem', :'linux:release_gem']
