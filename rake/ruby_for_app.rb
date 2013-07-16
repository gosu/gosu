RVM_RUBY         = "ruby-2.0.0-p247"
INTERNAL_VERSION = "2.0.0"
RUBY_DYLIB       = "libruby.#{INTERNAL_VERSION}.dylib"
RUBY_DYLIB_ID    = "@executable_path/../Frameworks/#{RUBY_DYLIB}"
TARGET_ROOT      = "mac/Ruby"
SOURCE_ROOT      = "#{ENV['HOME']}/.rvm/rubies/#{RVM_RUBY}"
GEM_ROOT         = "#{ENV['HOME']}/.rvm/gems/#{RVM_RUBY}/gems"
ALL_PLATFORMS    = [:ppc, :i386, :x86_64]
LIB_KILLLIST     = %w(README irb rake* rdoc* *ubygems* readline* tcltk* tk* tcltklib* rss* *-darwin*)
GEMS             = %w(gosu texplay chipmunk ruby-opengl2)

# Just to abbreviate the CFLAGS
SDK_10_5 = '/Developer/SDKs/MacOSX10.5.sdk'

CFLAGS           = {
  :ppc    => "''",
  :i386   => "'-isysroot #{SDK_10_5} -mmacosx-version-min=10.5 -I#{SDK_10_5}/usr/lib/gcc/i686-apple-darwin10/4.0.1/include -I#{SDK_10_5}/usr/include/c++/4.0.0 -I#{SDK_10_5}/usr/include/c++/4.0.0/i686-apple-darwin9'",
  :x86_64 => "'-include #{File.expand_path(TARGET_ROOT)}/define_environ.h'"
}
BUILD            = {
  :ppc    => %(powerpc-apple-darwin9.0),
  :i386   => %(i686-apple-darwin9.0),
  :x86_64 => %(x86_64-apple-darwin10.0),
}

def merge_lib source_file, target_file
  sh "install_name_tool -change #{SOURCE_ROOT}/lib/#{RUBY_DYLIB} #{RUBY_DYLIB_ID} #{source_file}"
  if File.exist? target_file then
    sh "lipo #{source_file} #{target_file} -create -output #{target_file}"
  else
    sh "cp #{source_file} #{target_file}"
  end
end

namespace :ruby_for_app do
  ALL_PLATFORMS.each do |platform|
    task platform.to_sym do
      mkdir_p "#{TARGET_ROOT}/include"
      mkdir_p "#{TARGET_ROOT}/lib"
      
      # Let RVM install the correct Ruby
      sh "env RVM_RUBY=#{RVM_RUBY} RVM_ARCH=#{platform} " +
         "    RVM_BUILD=#{BUILD[platform]} RVM_CFLAGS=#{CFLAGS[platform]} " +
         "    RVM_GEMS='#{GEMS.join(' ')}' " +
         "    bash #{TARGET_ROOT}/install_rvm_ruby.sh"
      
      # Copy headers
      sh "cp -R #{SOURCE_ROOT}/include/ruby*/* #{TARGET_ROOT}/include/"
      # Copy rbconfig
      sh "cp #{TARGET_ROOT}/rbconfig.rb #{TARGET_ROOT}/lib/rbconfig.rb"
      # Rename platform-specific folder so Xcode will find it
      sh "mv #{TARGET_ROOT}/include/*-darwin* #{TARGET_ROOT}/include/#{platform}"
      
      # Copy Ruby libraries
      sh "cp -R #{SOURCE_ROOT}/lib/ruby/#{INTERNAL_VERSION}/* #{TARGET_ROOT}/lib"
      # Merge libruby with existing platforms
      # (Yes, this will bork the installation in rvm)
      source_file = "#{SOURCE_ROOT}/lib/#{RUBY_DYLIB}"
      target_file = "#{TARGET_ROOT}/#{RUBY_DYLIB}"
      sh "install_name_tool -id #{RUBY_DYLIB_ID} #{source_file}"
      merge_lib source_file, target_file
      
      # Merge binary libraries
      Dir["#{SOURCE_ROOT}/lib/ruby/#{INTERNAL_VERSION}/*-darwin*/**/*.bundle"].each do |source_file|
        target_file = source_file.dup
        target_file["#{SOURCE_ROOT}/lib/ruby/#{INTERNAL_VERSION}/"] = ""
        target_file[/^[^\/]*\//] = ""
        target_file = "#{TARGET_ROOT}/lib/#{target_file}"
        mkdir_p File.dirname(target_file)
        merge_lib source_file, target_file
      end
      
      # Merge gems
      GEMS.each do |gem_name|
        gem_lib = Dir["#{GEM_ROOT}/#{gem_name}-*/lib"].first
        Dir["#{gem_lib}/**/*.rb"].each do |ruby_file|
          target_file = ruby_file.dup
          target_file[gem_lib] = "#{TARGET_ROOT}/lib"
          mkdir_p File.dirname(target_file)
          sh "cp #{ruby_file} #{target_file}"
        end

        Dir["#{gem_lib}/**/*.bundle"].each do |ext_file|
          target_file = ext_file.dup
          target_file[gem_lib] = "#{TARGET_ROOT}/lib"
          mkdir_p File.dirname(target_file)
          merge_lib ext_file, target_file
        end
      end

      LIB_KILLLIST.each do |item|
        sh "rm -rf #{TARGET_ROOT}/lib/#{item}"
      end
    end
  end
  
  task :build => [:clean] + ALL_PLATFORMS
  
  task :clean do
    sh "rm -rf #{TARGET_ROOT}/#{RUBY_DYLIB}"
    sh "rm -rf #{TARGET_ROOT}/lib/*"
    sh "rm -rf #{TARGET_ROOT}/include/*"
  end
end
