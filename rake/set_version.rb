task :set_version do
  throw "GOSU_RELEASE_VERSION must be set" if GOSU_VERSION == "0.0.0"
  
  major, minor, patch = *GOSU_VERSION.split(".")
  
  cmake_lists = File.read("cmake/CMakeLists.txt")
  cmake_lists.sub! /(set\(GOSU_VERSION_MAJOR ")[^"]+("\))/, "\\1#{major}\\2"
  cmake_lists.sub! /(set\(GOSU_VERSION_MINOR ")[^"]+("\))/, "\\1#{minor}\\2"
  cmake_lists.sub! /(set\(GOSU_VERSION_PATCH ")[^"]+("\))/, "\\1#{patch}\\2"
  File.open("cmake/CMakeLists.txt", "w") do |file|
    file.write cmake_lists
  end
  
  File.open("Gosu/Version.hpp", "w") do |file|
    file.puts <<-EOF
#pragma once

#define GOSU_MAJOR_VERSION #{major}
#define GOSU_MINOR_VERSION #{minor}
#define GOSU_POINT_VERSION #{patch}

namespace Gosu
{
    extern const std::string VERSION;
    extern const std::string LICENSES;
}
    EOF
  end
end
