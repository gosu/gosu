namespace :win do
  WINDOWS_FILES = COMMON_CPP_FILES + FileList["Gosu/*.hpp", "lib{,64}/*.{dll,lib}"]
  WINDOWS_FILES.uniq!
  
  WINDOWS_ARCHIVE_FILENAME = "pkg/gosu-windows-#{GOSU_VERSION}.zip"
  
  desc "Build the archive #{WINDOWS_ARCHIVE_FILENAME}"
  task :archive => :set_version do
    zip WINDOWS_ARCHIVE_FILENAME, WINDOWS_FILES
  end
end

[32, 64].each do |bits|
  namespace :"win#{bits}" do
    task :gem => :set_version
    
    spec = Gem::Specification.new do |s|
      # Copy most fields from the "normal" Ruby gem's specification
      %w(name version license summary description author date email homepage
         required_ruby_version).each do |field|
        s.send "#{field}=", GEM_SPEC.send(field)
      end
      s.platform = "x#{bits == 32 ? 86 : bits}-mingw32"
      # Ruby frontend for C++ extension
      s.files += Dir["lib/**/*.rb"]
      # Ruby-specific binaries for C++ extension
      if bits == 32
        s.files += FileList["lib/?.?/gosu.so", "lib/*.dll"]
      else
        s.files += FileList["lib64/?.?/gosu.so", "lib64/*.dll"]
        s.required_ruby_version = ">= 2.1.0"
      end
    end
    
    Gem::PackageTask.new(spec) do
    end
    
    task :release_gem => :gem do
      sh "gem push pkg/gosu-#{GOSU_VERSION}-#{spec.platform}.gem"
    end
  end
end
