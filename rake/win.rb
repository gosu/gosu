desc "Regenerates the ext/gosu-ffi/gosu-ffi.def file to include all FFI symbols"
task :deffile do
  symbols = Dir["ffi/*.cpp"].map do |header|
    IO.read(header).scan(/GOSU_FFI_API [A-Za-z0-9_ *]+ (Gosu_[^( ]+)/)
  end
  symbols << "Init_gosu"

  File.open("ext/gosu-ffi/gosu-ffi.def", "w") do |f|
    f.write "EXPORTS\n"
    f.write symbols.flatten.sort.uniq.join("\n")
    f.write "\n"
  end
end

desc "Updates the DEF file"
task :win => :deffile
