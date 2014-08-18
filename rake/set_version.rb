LICENSES = <<-EOF
  This software uses the following third-party libraries:
  
  Gosu, http://www.libgosu.org, MIT License, http://opensource.org/licenses/MIT
  SDL 2, http://www.libsdl.org, MIT License, http://opensource.org/licenses/MIT
  FreeImage, http://freeimage.sourceforge.net, FreeImage Public License
  libogg & libvorbis, http://www.xiph.org, BSD License, 3-Clause Version, http://www.xiph.org/licenses/bsd
  libsndfile, http://www.mega-nerd.com/libsndfile, GNU LGPL 3, http://www.gnu.org/copyleft/lesser.html
  OpenAL Soft, http://kcat.strangesoft.net/openal.html, GNU LGPL 2, http://www.gnu.org/licenses/old-licenses/lgpl-2.0.html
EOF

task :set_version do
  throw "GOSU_RELEASE_VERSION must be set" if GOSU_VERSION == '0.0.0'
  
  components = GOSU_VERSION.split('.')
  
  File.open('cmake/CMakeLists.txt', 'w') do |file|
    file.puts <<-EOF
SET(GOSU_VERSION_MAJOR "#{components[0]}")
SET(GOSU_VERSION_MINOR "#{components[1]}")
SET(GOSU_VERSION_PATCH "#{components[2]}")
#{File.readlines('cmake/CMakeLists.txt.in').map do |line|
        "#{line.chomp}\n"
        end.join()}
    EOF
  end
  
  File.open('Gosu/Version.hpp', 'w') do |file|
    file.puts <<-EOF
#ifndef GOSU_VERSION_HPP
#define GOSU_VERSION_HPP

#define GOSU_MAJOR_VERSION #{components[0]}
#define GOSU_MINOR_VERSION #{components[1]}
#define GOSU_POINT_VERSION #{components[2]}
#define GOSU_VERSION "#{GOSU_VERSION}"

#define GOSU_COPYRIGHT_NOTICE \\
#{LICENSES.split("\n").map { |line| "    \"#{line.gsub(/^ */, '')}\\n\"" }.join("\n")};
  
#endif
    EOF
  end
end
