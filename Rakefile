require 'rubygems'
require 'fileutils'
require 'date'
require 'rake/extensiontask'

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
  'reference/**/*.mdown',
]

GOSU_VERSION = ENV['GOSU_RELEASE_VERSION'] || '0.0.0'

def upload filename
  sh "scp -P 22000 '#{filename}' libgosu.org:/Library/WebServer/Documents/libgosu.org/downloads/"
end

def zip filename, files
  sh "zip #{filename} #{files.map { |fn| "'#{fn}'" }.join(' ')}"
end

def tar filename, files
  sh "COPYFILE_DISABLE=true tar -czf #{filename} #{files.to_a.uniq.map { |fn| "'#{fn}'" }.join(' ')}"
end

Dir.glob('./rake/*.rb').sort.each { |task| require task }

task :swig do
  sh "swig -c++ -ruby -autorename ext/gosu/gosu.swg"
  sh "patch --no-backup-if-mismatch -p0 <ext/gosu/gosu_SWIG_GC_PATCH.patch"
  sh "patch --no-backup-if-mismatch -p0 <ext/gosu/gosu_SWIG_RENAME_PATCH.patch"
end

task :release => [:'win:release', :'win:release_gem',
                  :'linux:release', :'linux:release_gem']
