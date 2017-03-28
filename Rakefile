require 'rubygems'
require 'fileutils'
require 'date'
require 'rake/extensiontask'
require 'rake/testtask'

COMMON_FILES = FileList[
  '.yardopts',
  'COPYING',
  'README.md',
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
]

GOSU_VERSION = ENV['GOSU_RELEASE_VERSION'] || '0.0.0'

def zip filename, files
  if RUBY_PLATFORM =~ /mswin$|mingw32|mingw64|win32\-|\-win32/
    sh "7z a #{filename} #{files.map { |fn| "\"#{fn}\"" }.join(' ')}"
  else
    sh "zip #{filename} #{files.map { |fn| "'#{fn}'" }.join(' ')}"
  end
end

Dir.glob('./rake/*.rb').sort.each { |task| require task }

task :swig do
  sh "swig -c++ -ruby -autorename -o src/RubyGosu.cxx ext/gosu/gosu.i"
  sh "patch --no-backup-if-mismatch -p0 <ext/gosu/gosu_SWIG_RENAME_PATCH.patch"
  sh "patch --no-backup-if-mismatch -p0 <ext/gosu/gosu_SWIG_UTF8_PATCH.patch"
  sh "patch --no-backup-if-mismatch -p0 <ext/gosu/gosu_SWIG_STRING_PATCH.patch"
end

task :update_rdoc do
  sh "yardoc"
  sh "scp -r doc/* #{ENV['PROJECTS_HOST']}:#{ENV['PROJECTS_ROOT']}/libgosu.org/rdoc"
  sh "rm -rf doc/*"
end

task :update_doxygen do
  sh "ssh #{ENV['PROJECTS_HOST']} 'cd #{ENV['PROJECTS_ROOT']}/libgosu.org/ && " +
       "svn checkout https://github.com/gosu/gosu/trunk/Gosu && PATH=../doxygen/bin:$PATH doxygen'"
end

Rake::TestTask.new do |t|
  t.verbose = true
  t.warning = true
  # On Windows, we want to require everything from the 'lib' directory directly.
  # On UNIX systems, the tests should be run after installing the gosu gem instead (ignore ./lib).
  t.libs = [] unless RUBY_PLATFORM =~ /mswin$|mingw32|mingw64|win32\-|\-win32/
end

task :test_interactive do
  ENV['GOSU_TEST_INTERACTIVE'] = "true"
  Rake::Task["test"].invoke
end