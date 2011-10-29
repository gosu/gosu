# TODO Support for offset_x etc. (module methods)
# TODO add constructor snippets from initialize method
# TODO huge Window / game snippet

# TODO use YARD for Gosu and use its output here instead of hacking around
# (But but but it was fun!!)
# TODO use ruby-plist gem to generate all this
# (But but but it was so mind bending!!)
def for_each_gosu_method
  File.read('reference/gosu.rb').scan(/\n  class ([A-Za-z_]+)\n((    .*\n)*)/) do
    class_name, class_body = $1, $2
    
    next if %w(Color GLTexInfo).include? class_name
    
    class_body.scan(/def ([a-z_.]+)(\(.+\))/) do
      method_name = $1
      args = $2[1..-2].split(', ')
      
      # Ignore methods whose previous comment starts with DEPRECATED
      if class_body =~ /# DEPRECATED.*\n    def #{method_name}\(/
        puts "Skipped #{class_name}##{method_name} (deprecated)"
        next
      end
      
      # Support for static methods
      # e.g. self.load_tiles<tab> => Image.load_tiles<tab>
      if class_name != 'Window' then
        method_name.sub! "self.", "#{class_name}."
      else
        # Not in the window though
        method_name.sub! "self.", ""
      end
      
      # Ignore methods that are never called directly.
      next if %w(initialize button_up button_down).include? method_name
      
      if args.last[0, 1] == '&' then
        block_name = args.last[1..-1].gsub('_', ' ') + ' here'
        args.pop
      end
      
      yield class_name, method_name, args, block_name
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
  if not method_name.include? '.' and class_name != 'Window' then
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
  name_args = args.map { |arg| arg[/^[^=]+/] }
  name = "#{method_name}(#{name_args.join(', ')})"
  name = "#{class_name}##{name}" unless method_name.include? '.'
  
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
  
  task :build => :clean do
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

__END__

<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>mainMenu</key>
  <dict>
    <key>items</key>
    <array>
      <string>0D9B9192-AAC6-47A0-A690-3072FCD6E362</string>
    </array>
    <key>submenus</key>
    <dict>
      <key>0D9B9192-AAC6-47A0-A690-3072FCD6E362</key>
      <dict>
        <key>items</key>
        <array>
          <string>59D6B856-4C3C-4DBF-B7F3-8BBE60572135</string>
          <string>------------------------------------</string>
        </array>
        <key>name</key>
        <string>Methods</string>
      </dict>
      <key>59D6B856-4C3C-4DBF-B7F3-8BBE60572135</key>
      <dict>
        <key>items</key>
        <array>
          <string>B1068F43-06A2-4BBB-92C7-C270B9727461</string>
        </array>
        <key>name</key>
        <string>Utility functions</string>
      </dict>
    </dict>
  </dict>
  <key>name</key>
  <string>Ruby/Gosu</string>
  <key>uuid</key>
  <string>B6DAED16-66D0-454E-9F2D-A599A2B809CD</string>
</dict>
</plist>
