LICENSES = <<-EOF
  This software uses the following third-party libraries:
  
  Gosu, http://www.libgosu.org, MIT License, http://opensource.org/licenses/MIT
  SDL 2, http://www.libsdl.org, MIT License, http://opensource.org/licenses/MIT
  libsndfile, http://www.mega-nerd.com/libsndfile, GNU LGPL 3, http://www.gnu.org/copyleft/lesser.html
  OpenAL Soft, http://kcat.strangesoft.net/openal.html, GNU LGPL 2, http://www.gnu.org/licenses/old-licenses/lgpl-2.0.html
EOF

task :set_version do
  throw "GOSU_RELEASE_VERSION must be set" if GOSU_VERSION == '0.0.0'
  
  major, minor, patch = *GOSU_VERSION.split('.')
  
  cmake_lists = File.read('cmake/CMakeLists.txt')
  cmake_lists.sub! /(set\(GOSU_VERSION_MAJOR ")[^"]+("\))/, "\\1#{major}\\2"
  cmake_lists.sub! /(set\(GOSU_VERSION_MINOR ")[^"]+("\))/, "\\1#{minor}\\2"
  cmake_lists.sub! /(set\(GOSU_VERSION_PATCH ")[^"]+("\))/, "\\1#{patch}\\2"
  File.open('cmake/CMakeLists.txt', 'w') do |file|
    file.write cmake_lists
  end
  
  File.open('Gosu/Version.hpp', 'w') do |file|
    file.puts <<-EOF
#ifndef GOSU_VERSION_HPP
#define GOSU_VERSION_HPP

#define GOSU_MAJOR_VERSION #{major}
#define GOSU_MINOR_VERSION #{minor}
#define GOSU_POINT_VERSION #{patch}
#define GOSU_VERSION "#{GOSU_VERSION}"

#define GOSU_COPYRIGHT_NOTICE \\
#{LICENSES.split("\n").map { |line| "    \"#{line.gsub(/^ */, '')}\\n\"" }.join(" \\\n")}
  
#endif
    EOF
  end
end
