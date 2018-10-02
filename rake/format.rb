desc "Verify that all C++ code is formatted correctly (and automatically fix some issues)"
task :format do
  Dir.glob("{Gosu,src,examples}/**/*.{hpp,cpp}") do |filename|
    lines = File.readlines(filename)
    last_indent = 0
    line_no = lines.count
    lines.reverse_each do |line|
      # No tabs, ever.
      line.gsub!("\t", "    ")
      
      # Same for \r.
      line.gsub!("\r", "")
      
      # No trailing space in non-empty lines.
      line.gsub!(/([^ ]) +$/) { $1 }

      # In accordance with the C++ Core Guidelines: https://github.com/isocpp/CppCoreGuidelines
      line.gsub!("template <", "template<")
      
      # Space after control flow operators.
      line.gsub!(" if(",     " if (")
      line.gsub!(" for(",    " for (")
      line.gsub!(" while(",  " while (")
      # assert counts as control flow (to me, anyway :P)
      line.gsub!(" assert(", " assert (")
      
      # Some people prefer a space before ! (me too), but it's hard to enforce with a regex.
      # => Consistently remove it instead.
      line.gsub!("! ", "!") unless line =~ /^ *\/\//
      
      # std::function<void(int)> => std::function<void (int)>
      line.gsub!(/function<([^ ]+)\(/) { "function<#$1 (" }
      
      location = "#{filename}:#{line_no}"
      
      case line
      when /^ +$/
        # If a line only contains whitespace, it must match the indentation of the code around it.
        # Lines that just contain random whitespace are reduced to empty lines.
        line.replace("\n") if line.chomp.length != last_indent
      when /^ *=/
        warn("#{location}: Lines should never start with the assignment operator")
      when /^ *if [^\n]+\)$/
        warn("#{location}: single-line if statements should be on one line")
      when /^ *(for|while) [^\n]+\)$/
        warn("#{location}: for/while always need braces")
      when /} else/
        warn("#{location}: else must always be at the start of the line")
      when /^ *else$/
        warn("#{location}: if-else must always have braces")
      when /@autoreleasepool$/
        warn("#{location}: @autoreleasepool must be followed by { on same line")
      end
      
      unless line =~ /^ *#/ # Ignore the indentation of preprocessor directives.
        last_indent = line[/^ */].length
        last_indent += 4 if line =~ /^ *(public|protected|private)\:$/
      end
      
      line_no -= 1
    end
    File.open(filename, "w") { |io| io.write lines.join }
  end
end
