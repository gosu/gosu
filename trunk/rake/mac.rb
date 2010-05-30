namespace :mac do
  desc "Build Gosu.framework"
  task :cpp => [:version, :cpp_docs] do
    sh "cd mac && xcodebuild -project Gosu.xcodeproj -target Gosu -configuration Release"
  end

  desc "Build lib/gosu.for_1_8.bundle, lib/gosu.for_1_9.bundle and RubyGosu App.app"
  task :ruby => [:version, :ruby_docs] do
    sh "cd mac && xcodebuild -project Gosu.xcodeproj -target 'RubyGosu Core' -configuration 'Release'"
    sh "cd mac && xcodebuild -project Gosu.xcodeproj -target 'RubyGosu Core' -configuration 'Release with 1.9'"
    sh "cd mac && xcodebuild -project Gosu.xcodeproj -target 'RubyGosu App' -configuration 'Release with 1.9'"
  end

  MAC_ARCHIVE_FILENAME = "public/gosu-mac-#{GOSU_VERSION}.tar.gz"

  desc "Build the archive #{MAC_ARCHIVE_FILENAME}"
  task :archive => [:cpp, :version] do
    files = COMMON_CPP_FILES + FileList['Gosu.framework/**/*']
    sh "tar -czf #{MAC_ARCHIVE_FILENAME} #{files.map { |filename| "'#{filename}'" }.join(' ')}"
  end
  
  MAC_WRAPPER_FILENAME = "public/gosu-mac-wrapper-#{GOSU_VERSION}.tar.gz"
  
  desc "Publish RubyGosu App.app"
  task :app_wrapper => [:cpp, :ruby, :version] do
    files = FileList['RubyGosu App.app/**/*']
    sh "tar -czf #{MAC_WRAPPER_FILENAME} #{files.map { |filename| "'#{filename}'" }.join(' ')}"
  end

  desc "Releases the archive #{MAC_ARCHIVE_FILENAME} and #{MAC_WRAPPER_FILENAME} on GoogleCode"
  task :release => [:archive, :app_wrapper] do
    sh "./googlecode_upload.py --summary=\"Gosu #{GOSU_VERSION}, compiled for Mac OS X (C++)\"" +
       " --project=gosu --user=julianraschke --labels=\"Featured,Type-Archive,OpSys-OSX\" #{MAC_ARCHIVE_FILENAME}"
    sh "./googlecode_upload.py --summary=\"Self-contained template app, containing Gosu #{GOSU_VERSION} and Ruby 1.9\"" +
       " --project=gosu --user=julianraschke --labels=\"Featured,Type-Archive,OpSys-OSX\" #{MAC_WRAPPER_FILENAME}"
  end

  task :gem => [:ruby, :version]

  MAC_SPEC = Gem::Specification.new do |s|
    apply_gemspec_defaults s
    s.platform = 'universal-darwin'
    s.files = COMMON_RUBY_FILES + FileList['lib/gosu.*.bundle']
  end
  Rake::GemPackageTask.new(MAC_SPEC) { |t| t.package_dir = 'public' }
  
  task :release_gem => :gem do
    sh "gem push public/gosu-#{GOSU_VERSION}-universal-darwin.gem"
  end
end
