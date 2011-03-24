namespace :mac do
  FRAMEWORK_FILENAME = "Gosu.framework"
  file FRAMEWORK_FILENAME => [:version, :cpp_docs] do
    sh "cd mac && xcodebuild -project Gosu.xcodeproj -target Gosu -configuration Release"
  end

  desc "Build lib/gosu.for_1_8.bundle, lib/gosu.for_1_9.bundle and RubyGosu App.app"
  task :ruby => [:version, :ruby_docs] do
    sh "cd mac && xcodebuild -project Gosu.xcodeproj -target 'RubyGosu Core' -configuration 'Release'"
    sh "cd mac && xcodebuild -project Gosu.xcodeproj -target 'RubyGosu Core' -configuration 'Release with 1.9'"
    sh "cd mac && xcodebuild -project Gosu.xcodeproj -target 'RubyGosu App' -configuration 'Release with 1.9'"
  end
  
  MAC_ARCHIVE_FILENAME = "public/gosu-mac-#{GOSU_VERSION}.tar.gz"
  
  file MAC_ARCHIVE_FILENAME => [FRAMEWORK_FILENAME, :version] do
    files = COMMON_CPP_FILES + FileList['Gosu.framework/**/*']
    sh "tar -czf #{MAC_ARCHIVE_FILENAME} #{files.map { |filename| "'#{filename}'" }.join(' ')}"
  end
  
  MAC_WRAPPER_FILENAME = "public/gosu-mac-wrapper-#{GOSU_VERSION}.tar.gz"
  
  desc "Publish RubyGosu App.app"
  task :app_wrapper => [FRAMEWORK_FILENAME, :ruby, :version] do
    files = FileList['RubyGosu App.app/**/*']
    sh "tar -czf #{MAC_WRAPPER_FILENAME} #{files.map { |filename| "'#{filename}'" }.join(' ')}"
  end

  desc "Releases the archive #{MAC_ARCHIVE_FILENAME} on GitHub"
  task :release => [MAC_ARCHIVE_FILENAME, 'rake/upload'] do
    sh "rake/upload #{MAC_ARCHIVE_FILENAME} '' 'Mac C++ package'"
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
