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
    %w(name version summary description author date email homepage).each do |field|
      s.send "#{field}=", GEM_SPEC.send(field)
    end
    s.platform = 'x86-mingw32'
    # Ruby frontend for C++ extension
    s.files += Dir["lib/**/*.rb"]
    # Ruby-specific binaries for C++ extension
    s.files += FileList['lib/?.?/gosu.so', 'lib/*.dll']
    # C++ Gosu header files, for compiling the gem & for using inline C++ from Ruby
    s.files += Dir["Gosu/*.hpp"]
    # Examples
    s.files += Dir["examples/*.rb"] + Dir["examples/media/**/*"]
  end

  Gem::PackageTask.new(WINDOWS_SPEC) do |pkg|
  end
  
  task :release_gem => :gem do
    sh "gem push pkg/gosu-#{GOSU_VERSION}-#{WINDOWS_SPEC.platform}.gem"
  end
end
