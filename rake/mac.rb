namespace :mac do
  ALTERNATIVE_XCODE = '/Developer-3.2.6/usr/bin/'
  # Prefer Xcode 3.2.6 if on Lion
  xcb_prefix = ALTERNATIVE_XCODE if File.directory? ALTERNATIVE_XCODE

  FRAMEWORK_FILENAME = "Gosu.framework"
  file FRAMEWORK_FILENAME => [:set_version, :cpp_docs] do
    sh "cd mac && #{xcb_prefix}xcodebuild -project Gosu.xcodeproj -target Gosu -configuration Release"
  end
  
  desc "Build lib/gosu.for_1_8.bundle, lib/gosu.for_1_9.bundle and RubyGosu App.app"
  task :ruby => [:set_version, :ruby_docs] do
    sh "cd mac && #{xcb_prefix}xcodebuild -project Gosu.xcodeproj -target 'RubyGosu Core' -configuration 'Release'"
    sh "cd mac && #{xcb_prefix}xcodebuild -project Gosu.xcodeproj -target 'RubyGosu Core' -configuration 'Release with 1.9'"
    sh "cd mac && #{xcb_prefix}xcodebuild -project Gosu.xcodeproj -target 'RubyGosu App'  -configuration 'Release with 1.9'"
  end
  
  MAC_ARCHIVE_FILENAME = "pkg/gosu-mac-#{GOSU_VERSION}.tar.gz"
  
  file MAC_ARCHIVE_FILENAME => [FRAMEWORK_FILENAME, :set_version] do
    files = COMMON_CPP_FILES + FileList['Gosu.framework/**/*']
    tar MAC_ARCHIVE_FILENAME, files
  end
  
  MAC_WRAPPER_FILENAME = "pkg/gosu-mac-wrapper-#{GOSU_VERSION}.tar.gz"
  
  desc "Publish RubyGosu App.app"
  task :app_wrapper => [FRAMEWORK_FILENAME, :ruby, :set_version] do
    tar MAC_WRAPPER_FILENAME, 'RubyGosu App.app'
    upload MAC_WRAPPER_FILENAME
  end

  desc "Releases the archive #{MAC_ARCHIVE_FILENAME}"
  task :release => MAC_ARCHIVE_FILENAME do
    upload MAC_ARCHIVE_FILENAME
  end

  task :gem => [:ruby, :set_version]

  MAC_SPEC = Gem::Specification.new do |s|
    apply_gemspec_defaults s
    s.platform = 'universal-darwin'
    s.files = COMMON_RUBY_FILES + FileList['lib/gosu.*.bundle']
  end
  Gem::PackageTask.new(MAC_SPEC)
  
  task :release_gem => :gem do
    sh "gem push pkg/gosu-#{GOSU_VERSION}-universal-darwin.gem"
  end
end
