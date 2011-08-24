namespace :win do
  WINDOWS_ARCHIVE_FILENAME = "pkg/gosu-windows-#{GOSU_VERSION}.zip"
  
  desc "Build the archive #{WINDOWS_ARCHIVE_FILENAME}"
  task :archive => :set_version do
    files = COMMON_CPP_FILES
    files += FileList['Gosu/*.hpp', 'lib/*.lib', 'lib/*.dll']
    # Not yet supported in C++
    files -= ['lib/FreeImage.dll']
    zip WINDOWS_ARCHIVE_FILENAME, files
  end

  desc "Releases the archive #{WINDOWS_ARCHIVE_FILENAME}"
  task :release => :archive do
    upload WINDOWS_ARCHIVE_FILENAME
  end

  task :gem => :set_version

  WINDOWS_SPEC = Gem::Specification.new do |s|
    apply_gemspec_defaults s
    s.platform = 'i386-mingw32'
    s.files = COMMON_RUBY_FILES
    s.files += FileList['lib/gosu.for_*.so', 'lib/*.dll']
  end
  Gem::PackageTask.new(WINDOWS_SPEC)
  
  task :release_gem => :gem do
    sh "gem push pkg/gosu-#{GOSU_VERSION}-x86-mingw32.gem"
  end
end
