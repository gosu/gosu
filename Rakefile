require "rubygems"
require "fileutils"
require "date"
require "rake/extensiontask"
require "rake/testtask"

GOSU_VERSION = ENV["GOSU_RELEASE_VERSION"] || "0.0.0"

Dir.glob("./rake/*.rb").sort.each { |task| require task }

desc "Update the C++ reference on libgosu.org (needs SSH access)"
task :update_doxygen do
  sh "ssh #{ENV["PROJECTS_HOST"]} 'cd #{ENV["PROJECTS_ROOT"]}/libgosu.org/ && " +
       "svn checkout https://github.com/gosu/gosu/trunk/include/Gosu && PATH=../doxygen/bin:$PATH doxygen'"
end

Rake::TestTask.new do |t|
  t.verbose = true
  t.warning = true
  # The tests should be run after installing the gosu gem (ignore ./lib).
  t.libs = []
end

task :test => :compile

desc "Run all tests, even those that require human input"
task :test_interactive do
  ENV["GOSU_TEST_INTERACTIVE"] = "true"
  Rake::Task["test"].invoke
end
