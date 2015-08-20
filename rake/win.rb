namespace :win do
  WINDOWS_ARCHIVE_FILENAME = "pkg/gosu-windows-#{GOSU_VERSION}.zip"
  
  desc "Build the archive #{WINDOWS_ARCHIVE_FILENAME}"
  task :archive => :set_version do
    files = COMMON_CPP_FILES
    files += FileList['Gosu/*.hpp', 'lib{,64}/*.{dll,lib}']
    zip WINDOWS_ARCHIVE_FILENAME, files
  end
  
  desc "Releases the archive #{WINDOWS_ARCHIVE_FILENAME}"
  task :release => :archive do
    sh "scp '#{WINDOWS_ARCHIVE_FILENAME}' #{ENV['PROJECTS_HOST']}:#{ENV['PROJECTS_ROOT']}/libgosu.org/downloads/"
  end
end

[32, 64].each do |bits|
  namespace :"win#{bits}" do
    task :gem => :set_version
    
    spec = Gem::Specification.new do |s|
      # Copy most fields from the 'normal' Ruby gem's specification
      %w(name version summary description author date email homepage).each do |field|
        s.send "#{field}=", GEM_SPEC.send(field)
      end
      s.platform = "x#{bits == 32 ? 86 : bits}-mingw32"
      # Ruby frontend for C++ extension
      s.files += Dir["lib/**/*.rb"]
      # Ruby-specific binaries for C++ extension
      if bits == 32 then
        s.files += FileList['lib/?.?/gosu.so', 'lib/*.dll']
      else
        s.files += FileList['lib64/?.?/gosu.so', 'lib64/*.dll']
        s.required_ruby_version = '>= 2.1.0'
      end
    end
    
    Gem::PackageTask.new(spec) do
    end
    
    task :release_gem => :gem do
      sh "gem push pkg/gosu-#{GOSU_VERSION}-#{spec.platform}.gem"
    end
  end
end
