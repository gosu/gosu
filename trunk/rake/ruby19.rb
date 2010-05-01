RUBY_DYLIB    = "libruby.1.9.1.dylib"
TARGET_ROOT   = "mac/Ruby"
SOURCE_ROOT   = "#{TARGET_ROOT}/source"
ALL_PLATFORMS = [:ppc, :i386, :x86_64]

namespace :ruby19 do
  ALL_PLATFORMS.each do |platform|
    task platform.to_sym do
      sh "bash #{TARGET_ROOT}/#{platform}.sh"

      source_file = "#{TARGET_ROOT}/#{platform}/#{RUBY_DYLIB}"
      target_file = "#{TARGET_ROOT}/lib/#{RUBY_DYLIB}"
      
      if File.exist? target_file then
        sh "lipo #{source_file} #{target_file} -create #{target_file}"
      else
        sh "cp #{source_file} #{target_file}"
      end
    end
  end
  
  task :copy_headers do
    sh "cp -R #{SOURCE_ROOT}/include #{TARGET_ROOT}"
  end
  
  task :build => [:copy_headers] + ALL_PLATFORMS do
    # yessir
  end
end
