task :set_version do
  throw "GOSU_RELEASE_VERSION must be set" if GOSU_VERSION == '0.0.0'

  components = GOSU_VERSION.split('.')
  
  File.open('Gosu/Version.hpp', 'w') do |file|
    file.puts <<-EOF
#ifndef GOSU_VERSION_HPP
#define GOSU_VERSION_HPP

#define GOSU_MAJOR_VERSION #{components[0]}
#define GOSU_MINOR_VERSION #{components[1]}
#define GOSU_POINT_VERSION #{components[2]}
#define GOSU_VERSION "#{GOSU_VERSION}"

#define GOSU_COPYRIGHT_NOTICE \\
#{Dir['dependencies/*.license'].map do |fn|
  File.readlines(fn).map do |line|
    "  \"#{line.chomp}\""
  end.join(" \\\n").gsub("  \"\"", "  \"\\n\\n\"")
end.join(" \\\n\\\n")}
  
#endif
    EOF
  end
end
