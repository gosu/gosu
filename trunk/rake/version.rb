task :version do
  throw "GOSU_RELEASE_VERSION must be set" if GOSU_VERSION == '0.0.0'

  components = GOSU_VERSION.split('.')
  
  File.open('Gosu/Version.hpp', 'w') do |file|
    file.puts '#ifndef GOSU_VERSION_HPP'
    file.puts '#define GOSU_VERSION_HPP'
    file.puts
    file.puts "#define GOSU_MAJOR_VERSION #{components[0]}"
    file.puts "#define GOSU_MINOR_VERSION #{components[1]}"
    file.puts "#define GOSU_POINT_VERSION #{components[2]}"
    file.puts "#define GOSU_VERSION \"#{GOSU_VERSION}\""
    file.puts
    file.puts '#endif'
  end
end
