RUBY_DYLIB    = "libruby.1.9.1.dylib"
TARGET_ROOT   = "mac/Ruby"
SOURCE_ROOT   = "#{TARGET_ROOT}/source"
ALL_PLATFORMS = [:ppc, :i386, :x86_64]

namespace :ruby19 do
  ALL_PLATFORMS.each do |platform|
    task platform.to_sym do
      sh "./#{TARGET_ROOT}/#{platform}.bash"
    end
  end
  
  # task :copy_headers => ALL_PLATFORMS do
  #   sh "cp -R #{SOURCE_ROOT}/include #{TARGET_ROOT}"
  # end
  # 
  # task :merge_libs => :target_root do
  #   sh "install_name_tool", "-change", "#{RVM_RUBY_ROOT}/lib/#{RUBY_DYLIB}",
  #      "@executable_path/../Frameworks/#{RUBY_DYLIB}", "#{RVM_RUBY_ROOT}/lib/#{RUBY_DYLIB}"
  #   if File.exist? "#{TARGET_ROOT}/lib/#{RUBY_DYLIB}" then
  #     sh "lipo #{RVM_RUBY_ROOT}/lib/#{RUBY_DYLIB} #{TARGET_ROOT}/lib/#{RUBY_DYLIB} -create " +
  #        "-ouput #{TARGET_ROOT}/lib/#{RUBY_DYLIB}"
  #   else
  #     sh "cp #{RVM_RUBY_ROOT}/lib/#{RUBY_DYLIB} #{TARGET_ROOT}/lib"
  #   end
  # end
end
