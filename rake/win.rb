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
