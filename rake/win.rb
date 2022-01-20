def update_msvc_project(path:, interface:, implementation:, dependencies: [])
  # Some header files are considered implementation details.
  headers = (dependencies + implementation + interface).grep(/\.(h|hpp)$/)
  sources = (dependencies + implementation).grep(/\.(c|cpp)$/)

  # The vcxproj file only cares about ClInclude (headers) vs ClCompile (sources).
  vcxproj = File.read(path)
  vcxproj.gsub! /(<ItemGroup>)(\r?\n\s*)<ClInclude.*?(\n\s*<\/ItemGroup>)/m do
    items = headers.map { |file| "#$2<ClInclude Include=\"..\\#{file.tr("/", "\\")}\" />" }
    [$1, *items, $3].join
  end
  vcxproj.gsub! /(<ItemGroup>)(\r?\n\s*)<ClCompile.*?(\n\s*<\/ItemGroup>)/m do
    items = sources.map { |file| "#$2<ClCompile Include=\"..\\#{file.tr("/", "\\")}\" />" }
    [$1, *items, $3].join
  end
  File.write(path, vcxproj)

  # The vcxproj.filters file is where headers and sources are sorted into folders ("filters").
  filters = File.read("#{path}.filters")
  filters.gsub! /(<ItemGroup>)(\r?\n\s*)<ClInclude.*?(\n\s*<\/ItemGroup>)/m do
    items = headers.map do |file|
      if interface.include? file
        filter = "Interface"
      elsif implementation.include? file
        filter = "Implementation"
      else
        filter = "Dependencies"
      end
      "#$2<ClInclude Include=\"..\\#{file.tr("/", "\\")}\">#$2  <Filter>#{filter}</Filter>#$2</ClInclude>"
    end
    [$1, *items, $3].join
  end
  filters.gsub! /(<ItemGroup>)(\r?\n\s*)<ClCompile.*?(\n\s*<\/ItemGroup>)/m do
    items = sources.map do |file|
      if interface.include? file
        filter = "Interface"
      elsif implementation.include? file
        filter = "Implementation"
      else
        filter = "Dependencies"
      end
      "#$2<ClCompile Include=\"..\\#{file.tr("/", "\\")}\">#$2  <Filter>#{filter}</Filter>#$2</ClCompile>"
    end
    [$1, *items, $3].join
  end
  File.write("#{path}.filters", filters)
end

desc "Updates the Visual Studio projects to include all C++ and C source files"
task :msvc do
  update_msvc_project(path: "windows/Gosu.vcxproj",
                      interface: FileList["include/**/*.hpp"],
                      implementation: FileList["src/**/*.{hpp,cpp}"],
                      dependencies: FileList["dependencies/**/*.{h,hpp,c,cpp}"])
  update_msvc_project(path: "windows/GosuFFI.vcxproj",
                      interface: FileList["ffi/*.h"],
                      implementation: FileList["ffi/*.cpp"])
end
