
# RDoc workarounds
class String
  def if_exists (item = nil)
    unless item
      self unless self =~ /(%(\w+)%)/
      "\nIF:#{$2}\n#{self}\nENDIF:#{$2}\n"
    else
      "\nIF:#{item}\n#{self}\nENDIF:#{item}\n"
    end
  end
  def loop(item)
    "\nSTART:#{item}\n#{self}\nEND:#{item}\n"
  end
end

module RDoc
  module Page

    puts "Allison 2 template (c) 2007 Cloudburst, LLC"

    require 'pathname'
    CACHE_DIR = File.expand_path(File.dirname(__FILE__) + "/../cache")
    Dir.mkdir(CACHE_DIR) unless File.exist?(CACHE_DIR)

    begin
      require 'rubygems'
      gem 'markaby', '>= 0.5'  
      require 'markaby'
      require 'base64'

      # Markaby page says Markaby is better in its own module
      module Allison
        
        FONTS = METHOD_LIST = SRC_PAGE = FILE_PAGE = CLASS_PAGE = ""
              
        FR_INDEX_BODY = "!INCLUDE!" # Who knows
        
        STYLE, JAVASCRIPT = ["css", "js"].map do |extension|
          s = File.open(File.dirname(__FILE__) + "/allison.#{extension}").read
          # Programmatic CSS
          if extension == "css"
            s_lines = s.split("\n")
            meths = []
            s_lines.collect! do |line|
              line = line.squeeze(" ").strip
              if line =~ /(\w+)/ and meths.include? $1
                line = instance_eval line
                # puts "Called method #{$1}"
              elsif line !~ /\*\/|\/\*/ and line =~ /(@.*|^def (\w+).*)/ 
                # printf "Evalled #{$1}"
                result = instance_eval $1
                # puts " to #{result.inspect}"
                result = (result.is_a?(Fixnum) ? result.to_s + "px" : result.to_s)
                line = (line == $1 ? "" : line.gsub($1, result))
                meths.push $2 if $2
              end
              line !~ /^\s*$|\s*^\/\*.*\*\/\s*$|\{|\}/ ? line + ";" : line
              end
            s = s_lines.join("\n")
          else
            # Do nothing; the Javascript is already fine
          end
          s
        end      
    
        INDEX = Markaby::Builder.new.xhtml_strict do
          head do
            title '%title%'
            link :rel => 'stylesheet', :type => 'text/css', :href => 'rdoc-style.css', :media => 'screen'
            tag! :meta, 'http-equiv' => 'refresh', 'content' => '0;url=%initial_page%'
          end
          body do
            div.container! do
              6.times {|n| div('', :class => "curve", :id => "preheader_curve_#{n}") }
              div.header! do
                span.title! do
                  p { '&nbsp;' }
                  h1 "Ruby Documentation"
                end
              end
              div.clear {}
              div.redirect! do
                a :href => '%initial_page%' do
                  h1 "Redirect"
                end
              end
            end
          end
        end.to_s
        
        FILE_INDEX = METHOD_INDEX = CLASS_INDEX = Markaby::Builder.new.capture do 
          a :href => '%href%' do
            self << '%name%'
            br
          end
        end.loop('entries')
                
        BODY = Markaby::Builder.new.xhtml_strict do
          head do
            title "%title%"
            link :rel => 'stylesheet', :type => 'text/css', :href => '%style_url%', :media => 'screen'
            script :type => 'text/javascript' do
              JAVASCRIPT
            end
          end
          body do 
            div.container! do
              6.times {|n| div('', :class => "curve", :id => "preheader_curve_#{n}") }
              div.header! do
                span do
                  h1.title! '%title%'.if_exists
                end
                self << "!INCLUDE!" # Always empty
              end
              div.clear {}
              div.left! do
                self << (div.navigation.darker.top.child_of! do 
                  # Ugh
                  h3 "Child of" 
                  self << "<span>\n#{"<a href='%par_url%'>".if_exists}%parent%#{"</a>".if_exists('par_url')}</span>"
                end).if_exists('parent')
                
                self << div.navigation.darker.top.defined_in! do 
                  h3('Defined in')
                  self << a('%full_path%', :href => '%full_path_url%').if_exists.loop('infiles')
                end.if_exists('infiles')
                
                ['includes', 'requires', 'methods'].each do |item|
                  self << div.navigation.top(:id => item) do
                    self << h3(item.capitalize)
                    self << "#{"<a href='%aref%'>".if_exists}%name%#{br}#{"</a>".if_exists('aref')}".if_exists('name').loop(item)
                  end.if_exists(item)
                end 
                
                div.spacer! ''
                
                # For the local AJAX includes
                ['class', 'file', 'method'].each do |item|
                  div.navigation.darker.index :id => "#{item}_wrapper" do
                   div.list_header do
                     h3 'All ' + (item == 'class' ? 'classes' : item + 's')
                   end                 
                   div.list_header_link do
                     a((item == 'method' ? 'Show...' : 'Hide...'), 
                        :id => "#{item}_link", :href => "#", 
                        :onclick=> "toggle('#{item}'); toggleText('#{item}_link'); return false;")
                   end
                   div.clear {}
                   div(:id => item) do
                     form do
                       label(:for => "filter_#{item}") { 'Filter:' + '&nbsp;' * 2 }
                       input '', :type => 'text', :id => "filter_#{item}", 
                                   :onKeyUp => "return filterList('#{item}', this.value, event);",
                                   :onKeyPress => "return disableSubmit(event);"
                     end
                   end
                  end
                end
              end            
              
              div.content! do
                self << capture do
                  div.description! do
                    '%description%'
                  end
                end.if_exists('description')
  
                self << capture do
                  self << h1 {a '%sectitle%', :name => '%secsequence%'}.if_exists('sectitle')
                  self << p {'%seccomment%'}.if_exists
    
                  self << capture do
                    h1 "Child modules and classes"
                    p '%classlist%'
                  end.if_exists('classlist')
  
                  ['constants', 'aliases', 'attributes'].each do |item|
                    self << capture do
                      h1(item.capitalize)
                      p do
                        table do
                          fields = %w[name value old_name new_name rw]
                          self << tr do
                            # Header row
                            if item == 'constants'
                              th 'Name'
                              th 'Value'
                            elsif item == 'aliases'
                              th 'Old name'
                              th 'New name'
                            elsif item == 'attributes'
                              th 'Name'
                              th 'Read/write?'
                            end
                          end
                          self << tr do
                            # Looped item rows
                            fields.each do |field|
                              if field !~ /desc/ 
                                self << td('%' + field + '%', :class => field =~ /^old|^name/ ? "highlight" : "normal").if_exists
                              else
                                self << td(('%' + field+ '%').if_exists)
                              end
                            end
                          end.loop(item)
                        end
                      end
                    end.if_exists(item)
                  end
  
                  self << capture do
                    h1('%type% %category% Methods')   
                    self << capture do
                      self << a.small(:name => '%aref%') {br}.if_exists
                      div.method_block do
                        h3 { "<a href='#%aref%'>".if_exists + '%callseq%'.if_exists + '%name%'.if_exists + '%params%'.if_exists + "</a>".if_exists('aref')}
                        self << '%m_desc%'.if_exists

                        self << capture do
                          p.source_link :id => '%aref%-show-link' do
                            a "Show source...", :id => '%aref%-link', :href => "#", 
                               :onclick=> "toggle('%aref%-source'); toggleText('%aref%-link'); return false;"
                          end
                          div.source :id => '%aref%-source' do
                              pre { '%sourcecode%' }
                            end
                          end.if_exists('sourcecode')
                        end
  
                    end.loop('methods').if_exists('methods')             
                  end.loop('method_list').if_exists('method_list')             
  
                end.loop('sections').if_exists('sections')
                
              end            
  
              div.footer!.clear do 
                self << Time.now.strftime("Generated on %b %d, %Y").gsub(' 0', ' ')
                self << " / Allison 2 &copy; 2007 "
                a "Cloudburst, LLC", :href => "http://cloudbur.st"
                br
                self << "slightly adapted for the Gosu project by Julian Raschke"
              end
            end                                      
          end          
                
        end.to_s                 
      end

    Allison.constants.each do |c| 
      eval "#{c} = Allison::#{c}" # Jump out of the namespace
      begin
        File.open("#{CACHE_DIR}/#{c}", 'w') do |f|
          f.puts eval(c) # Write cache
        end
      rescue Errno::EACCES => e
      end
    end     
    
    rescue LoadError => e
      # We don't have some dependency
      lib = (e.to_s[/(.*)\(/, 1] or e.to_s).split(" ").last.capitalize
      puts "Using cache (couldn't load '#{lib}')"
      Dir[CACHE_DIR + '/*'].each do |filename|
        eval("#{filename.split("/").last} = File.open(filename) {|s| s.read}")
      end
    end
      
  end 
  
end 
