RVM_RUBY      = "ruby-1.9.2-preview1"
RVM_RUBY_ROOT = "#{ENV['HOME']}/.rvm/rubies/#{RVM_RUBY}"
RUBY_DYLIB    = "libruby.1.9.1.dylib"
TARGET_ROOT   = "mac/Ruby"

namespace :ruby19 do
  task :target_root do
    sh "mkdir -p #{TARGET_ROOT}/lib"
  end
  
  task :reinstall do
    sh "rvm remove #{RVM_RUBY}"
    sh "rvm install #{RVM_RUBY} -C --enable-shared,--with-readline-dir=/usr/local,--build=x86_64-apple-darwin10"
  end

  task :copy_headers => :target_root do
    sh "cp -R #{RVM_RUBY_ROOT}/include #{TARGET_ROOT}"
  end

  task :merge_libs => :target_root do
    sh "cp -R #{RVM_RUBY_ROOT}/lib/#{RUBY_DYLIB} #{TARGET_ROOT}/lib"
    sh "install_name_tool -change #{RVM_RUBY_ROOT}/lib/#{RUBY_DYLIB} @executable_path/../Frameworks/#{RUBY_DYLIB} #{TARGET_ROOT}/lib/#{RUBY_DYLIB}"
  end
end
