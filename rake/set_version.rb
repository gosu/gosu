desc "Updates Version.hpp, CMakeLists.txt, and Gosu.podspec to match ENV['GOSU_RELEASE_VERSION']"
task :set_version do
  throw "GOSU_RELEASE_VERSION must be set" if GOSU_VERSION == "0.0.0"
  
  cmake_lists = File.read("CMakeLists.txt")
  cmake_lists.sub! /(project\(Gosu VERSION )[^)]+(\))/, "\\1#{GOSU_VERSION}\\2"
  File.open("CMakeLists.txt", "w") do |file|
    file.write cmake_lists
  end

  podspec = File.read("Gosu.podspec")
  podspec.sub! /(s\.version\s+=\s+\")[^"]+(")/, "\\1#{GOSU_VERSION}\\2"
  File.open("Gosu.podspec", "w") do |file|
    file.write podspec
  end

  major, minor, patch = *GOSU_VERSION.split(".")

  File.open("include/Gosu/Version.hpp", "w") do |file|
    file.puts <<-EOF
#pragma once

#include <string>

#define GOSU_MAJOR_VERSION #{major}
#define GOSU_MINOR_VERSION #{minor}
#define GOSU_POINT_VERSION #{patch}

namespace Gosu
{
    //! A string that contains the full version of the Gosu library, like "0.7.50" or "1.0.0.pre5".
    extern const std::string VERSION;
    
    //! A block of legal copy that your game is obliged to display somewhere.
    extern const std::string LICENSES;
}
    EOF
  end
end
