RUBY_DYLIB    = "libruby.1.9.1.dylib"
TARGET_ROOT   = "mac/Ruby"
SOURCE_ROOT   = "#{TARGET_ROOT}/source"
ALL_PLATFORMS = [:ppc, :i386, :x86_64]

def merge_lib source_file, target_file
  if File.exist? target_file then
    sh "lipo #{source_file} #{target_file} -create #{target_file}"
  else
    sh "cp #{source_file} #{target_file}"
  end
end

namespace :ruby19 do
  ALL_PLATFORMS.each do |platform|
    task platform.to_sym do
      # Compile Ruby and libruby
      sh "bash #{TARGET_ROOT}/#{platform}.sh"
      
      # Merge libruby with existing platforms
      source_file = "#{TARGET_ROOT}/#{platform}/#{RUBY_DYLIB}"
      target_file = "#{TARGET_ROOT}/lib/#{RUBY_DYLIB}"
      merge_lib source_file, target_file
    end
  end
  
  task :copy_headers do
    sh "cp -R #{SOURCE_ROOT}/include #{TARGET_ROOT}"
  end
  
  task :build => [:copy_headers] + ALL_PLATFORMS do
    # yessir
  end
  
  task :cleanup do
    ALL_PLATFORMS.each do |platform|
      sh "rm -rf #{TARGET_ROOT}/#{platform}"
    end
  end
end
