# TODO refactor (this was just superficially patched to use YARD)

# TODO use ruby-plist gem to generate all this
# (But but but it was so mind bending!!)

def for_each_gosu_method
  require 'yard'
  YARD::Registry.load!
  
  # Utility functions
  YARD::Registry.at("Gosu").meths.each do |meth|
    args = meth.parameters.map { |arg| arg.compact.join(' = ') }
    yield "Math Helpers", "Gosu::#{meth.name}", args, nil
  end
  
  # Methods and class methods
  YARD::Registry.all(:class).each do |cls|
    next if %w(Color GLTexInfo).include? cls.name
    
    cls.meths.each do |meth|
      # Ignore methods whose previous comment starts with @deprecated
      if meth.has_tag? :deprecated then
        puts "Skipped #{meth.path} (@deprecated)"
        next
      end
      
      # Ignore explicit and attr setters (methods that end in a =)
      next if meth.name.to_s[-1, 1] == '='
      
      # Ignore methods with only one argument
      next if meth.parameters.empty?
      
      method_name = meth.name.to_s
      method_name = "self.#{method_name}" if meth.scope == :class
      args = meth.parameters.map { |arg| arg.compact.join(' = ') }
      
      # Support for static methods
      # e.g. self.load_tiles<tab> => Image.load_tiles<tab>
      if cls.name != :Window then
        method_name.sub! "self.", "#{cls.name}."
      else
        # Not in the window though
        method_name.sub! "self.", ""
      end
      
      # Ignore methods that are never called directly.
      next if %w(button_up button_down).include? method_name
      
      if method_name == 'initialize' then
        next if cls.name == :Window # Nobody wants to call Window.new
        method_name = "#{cls.name}.new"
      end
      
      if args.last[0, 1] == '&' then
        block_name = args.last[1..-1].gsub('_', ' ') + ' here'
        args.pop
      end
      
      yield cls.name.to_s, method_name, args, block_name
    end
  end
end

def next_uuid
  gen = lambda { |n| (0...n).map { rand(16).to_s(16).upcase } }
  (gen[8] + %w(-) + gen[4] + %w(-) + gen[4] + %w(-) + gen[4] + %w(-) + gen[12]).join
end

BUNDLE_ROOT = "pkg/RubyGosu.tmbundle"
SNIPPET_ROOT = "#{BUNDLE_ROOT}/Snippets"

def build_snippet! class_name, method_name, args, block_name
  uuid = next_uuid
  
  # Add a leading dot to non-static triggers, except in Window
  trigger = method_name
  if not method_name[/\.|\:\:/] and class_name != 'Window' then
    trigger = ".#{trigger}"
  end
  
  content_args = (0...args.size).map { |i| "${#{i + 1}:#{args[i]}}" }
  if method_name =~ /draw/ then
    content = "#{trigger} #{content_args.join(', ')}"
  else
    content = "#{trigger}(#{content_args.join(', ')})"
  end
  if block_name then
    content = "#{content} do\n  ${#{args.size + 1}:# #{block_name}}\nend"
  end
  name_args = args.map { |arg| arg[/^[^ =]+/] }
  name = "#{method_name}(#{name_args.join(', ')})"
  name = "#{class_name}##{name}" unless method_name =~ /\.|\:\:/
  
  File.open("#{SNIPPET_ROOT}/#{uuid}.tmSnippet", "w+") do |io|
    io.puts <<-END.gsub(/^ {6}/, '')
      <?xml version="1.0" encoding="UTF-8"?>
      <!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
      <plist version="1.0">
      <dict>
        <key>content</key>
        <string>#{content}</string>
        <key>name</key>
        <string>#{name}</string>
        <key>scope</key>
        <string>source.ruby</string>
        <key>tabTrigger</key>
        <string>#{trigger}</string>
        <key>uuid</key>
        <string>#{uuid}</string>
      </dict>
      </plist>
    END
  end
  
  uuid
end

def build_plist! methods_of_classes
  class_uuids = Hash.new { |hash, key| hash[key] = next_uuid }
  
  File.open("#{BUNDLE_ROOT}/info.plist", "w+") do |io|
    io.puts <<-END.gsub(/^ {6}/, '')
      <?xml version="1.0" encoding="UTF-8"?>
      <!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
      <plist version="1.0">
      <dict>
        <key>mainMenu</key>
        <dict>
          <key>items</key>
          <array>
            #{methods_of_classes.keys.sort.map do |class_name|
              "<string>#{class_uuids[class_name]}</string>"
            end.join}
          </array>
          <key>submenus</key>
          <dict>
            #{methods_of_classes.keys.map do |class_name|
              <<-INNER_END.gsub(/^ {10}/, '')
                <key>#{class_uuids[class_name]}</key>
                <dict>
                  <key>name</key>
                  <string>#{class_name}</string>
                  <key>items</key>
                  <array>
                    #{methods_of_classes[class_name].map do |method_uuid|
                      "<string>#{method_uuid}</string>"
                    end.join}
                  </array>
                </dict>
              INNER_END
            end.join}
          </dict>
        </dict>
        <key>name</key>
        <string>RubyGosu</string>
        <key>uuid</key>
        <string>#{srand(20878642); "TODO make this cleaner"; next_uuid}</string>
      </dict>
      </plist>
    END
  end
end

namespace :tmbundle do
  task :clean do
    rm_rf "pkg/RubyGosu.tmbundle"
  end
  
  task :build => [:ruby_docs, :clean] do
    methods_of_classes = {}
    
    mkdir_p SNIPPET_ROOT
    
    for_each_gosu_method do |class_name, method_name, args, block_name|
      method_uuid = build_snippet!(class_name, method_name, args, block_name)
      
      methods_of_classes[class_name] ||= []
      methods_of_classes[class_name] << method_uuid
    end
    
    build_plist! methods_of_classes
  end
  
  task :install => :build do
    sh "open #{BUNDLE_ROOT}"
  end
end
