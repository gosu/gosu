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
  'examples/**/*.{cpp}',
  'examples/media/*',
  'reference/cpp/**/*',
]

COMMON_RUBY_FILES = COMMON_FILES + FileList[
  'Gosu/**/*',
  'lib/**/*.rb',
  'examples/*.rb',
  'examples/media/*',
  'reference/**/*.rb',
  'reference/**/*.mdown',
]

GOSU_VERSION = ENV['GOSU_RELEASE_VERSION'] || '0.0.0'

def zip filename, files
  sh "zip #{filename} #{files.map { |fn| "'#{fn}'" }.join(' ')}"
end

def tar filename, files
  sh "gtar -c --transform 's,^,gosu-#{GOSU_VERSION}/,' -zf #{filename} #{files.map { |fn| "'#{fn}'" }.join(' ')}"
end

Dir.glob('./rake/*.rb').sort.each { |task| require task }

task :swig do
  sh "swig -c++ -ruby -autorename ext/gosu/gosu.swg"
  sh "patch --no-backup-if-mismatch -p0 <ext/gosu/gosu_SWIG_GC_PATCH.patch"
  sh "patch --no-backup-if-mismatch -p0 <ext/gosu/gosu_SWIG_RENAME_PATCH.patch"
end

task :update_rdoc do
  sh "rm -rf doc/*"
  sh "yardoc"
  sh "scp -r doc/* #{ENV['PROJECTS_HOST']}:#{ENV['PROJECTS_ROOT']}/libgosu.org/rdoc"
end
