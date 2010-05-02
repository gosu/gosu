RUBY_DYLIB      = "libruby.1.9.1.dylib"
TARGET_ROOT     = "mac/Ruby"
SOURCE_ROOT     = "#{TARGET_ROOT}/source"
ALL_PLATFORMS   = [:ppc, :i386, :x86_64]
STDLIB_KILLLIST = %w(README irb rake* racc rdoc* *ubygems* cgi* readline* tkutil* tcltklib* rss*)

def merge_lib source_file, target_file
  if File.exist? target_file then
    sh "lipo #{source_file} #{target_file} -create -output #{target_file}"
  else
    sh "cp #{source_file} #{target_file}"
  end
end

namespace :ruby19 do
  ALL_PLATFORMS.each do |platform|
    task platform.to_sym do
      # Compile Ruby and libruby
      sh "bash #{TARGET_ROOT}/#{platform}.sh"
      
      # Copy platform specific config.h
      sh "mkdir -p #{TARGET_ROOT}/include/#{platform}/ruby"
      sh "cp #{TARGET_ROOT}/#{platform}/.ext/include/*/ruby/*.h #{TARGET_ROOT}/include/#{platform}/ruby/"
      
      # Merge libruby with existing platforms
      source_file = "#{TARGET_ROOT}/#{platform}/#{RUBY_DYLIB}"
      target_file = "#{TARGET_ROOT}/#{RUBY_DYLIB}"
      merge_lib source_file, target_file
      
      # Copy binary libraries
      Dir["#{TARGET_ROOT}/#{platform}/.ext/*darwin*/*.bundle"].each do |source_file|
        target_file = "#{TARGET_ROOT}/lib/#{File.basename(source_file)}"
        merge_lib source_file, target_file
      end
    end
  end
  
  task :copy_headers do
    sh "cp -R #{SOURCE_ROOT}/include #{TARGET_ROOT}"
  end
  
  task :copy_stdlib do
    sh "mkdir -p #{TARGET_ROOT}/lib"
    sh "cp -R #{SOURCE_ROOT}/lib/* #{TARGET_ROOT}/lib/"
    STDLIB_KILLLIST.each do |item|
      sh "rm -rf #{TARGET_ROOT}/lib/#{item}"
    end
  end
  
  task :build => [:cleanup, :copy_headers] + ALL_PLATFORMS do
    # yessir
  end
  
  task :cleanup do
    sh "rm -rf #{TARGET_ROOT}/lib/*"
    sh "rm -rf #{TARGET_ROOT}/include/*"
    ALL_PLATFORMS.each do |platform|
      sh "rm -rf #{TARGET_ROOT}/#{platform}"
    end
  end
end
