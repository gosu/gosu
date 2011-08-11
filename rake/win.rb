namespace :win do
  WINDOWS_ARCHIVE_FILENAME = "public/gosu-windows-#{GOSU_VERSION}.zip"
  
  desc "Build the archive #{WINDOWS_ARCHIVE_FILENAME}"
  task :archive => [:set_version] do
    files = COMMON_CPP_FILES + FileList['Gosu/*.hpp', 'lib/Gosu.lib', 'lib/GosuDebug.lib', 'lib/audiere.dll']
    zip WINDOWS_ARCHIVE_FILENAME, files
  end

  desc "Releases the archive #{WINDOWS_ARCHIVE_FILENAME} on GitHub"
  task :release => :archive do
    upload WINDOWS_ARCHIVE_FILENAME
  end

  task :gem => [:set_version]

  WINDOWS_SPEC = Gem::Specification.new do |s|
    apply_gemspec_defaults s
    s.platform = 'i386-mingw32'
    s.files = COMMON_RUBY_FILES + ['lib/gosu.for_1_8.so', 'lib/gosu.for_1_9.so', 'lib/libsndfile.dll', 'lib/OpenAL32.dll']
  end
  Gem::PackageTask.new(WINDOWS_SPEC) { |t| t.package_dir = 'public' }
  
  task :release_gem => :gem do
    sh "gem push public/gosu-#{GOSU_VERSION}-x86-mingw32.gem"
  end
end
