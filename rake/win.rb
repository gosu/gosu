namespace :win do
  WINDOWS_FILES = COMMON_CPP_FILES + FileList["include/Gosu/*.hpp", "lib{,64}/*.{dll,lib}"]
  WINDOWS_FILES.uniq!
  
  WINDOWS_ARCHIVE_FILENAME = "pkg/gosu-windows-#{GOSU_VERSION}.zip"
  
  desc "Build the archive #{WINDOWS_ARCHIVE_FILENAME}"
  task :archive => :set_version do
    zip WINDOWS_ARCHIVE_FILENAME, WINDOWS_FILES
  end
  
  desc "Updates the Visual Studio project to include all C++ and C source files"
  task :vcxproj do
    files = FileList["{src,dependencies}/**/*.{c,cpp}"]

    vcxproj = File.read "windows/Gosu.vcxproj"
    vcxproj.sub! /(<ItemGroup>)(\r?\n\s*)<ClCompile.*?(\n\s*<\/ItemGroup>)/m do
      items = files.map { |cpp| "#$2<ClCompile Include=\"..\\#{cpp.tr("/", "\\")}\" />" }
      [$1, *items, $3].join
    end
    File.write "windows/Gosu.vcxproj", vcxproj
    
    filters = File.read "windows/Gosu.vcxproj.filters"
    filters.sub! /(<ItemGroup>)(\r?\n\s*)<ClCompile.*?(\n\s*<\/ItemGroup>)/m do
      items = files.map { |cpp| "#$2<ClCompile Include=\"..\\#{cpp.tr("/", "\\")}\">#$2  <Filter>Implementation</Filter>#$2</ClCompile>" }
      [$1, *items, $3].join
    end
    File.write "windows/Gosu.vcxproj.filters", filters
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
      # Only provide prebuilt Windows gems for Ruby 2.3+.
      s.required_ruby_version = ">= 2.3.0"

      s.platform = "x#{bits == 32 ? 86 : bits}-mingw32"
      # Ruby frontend for C++ extension
      s.files += Dir["lib/**/*.rb"]
      # Ruby-specific binaries for C++ extension
      if bits == 32
        s.files += FileList["lib/?.?/gosu.so", "lib/*.dll"]
      else
        s.files += FileList["lib64/?.?/gosu.so", "lib64/*.dll"]
      end
    end
    
    Gem::PackageTask.new(spec) do
    end
    
    desc "Push the #{bits}-bit for Windows to rubygems.org"
    task :release_gem => :gem do
      sh "gem push pkg/gosu-#{GOSU_VERSION}-#{spec.platform}.gem"
    end
  end
end
