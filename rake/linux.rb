namespace :linux do
  LINUX_FILES = COMMON_CPP_FILES + COMMON_RUBY_FILES + FileList[
    'Gosu/**/*', 'GosuImpl/**/*',
    'linux/configure', 'linux/configure.ac', 'linux/Makefile.in'
  ]
  LINUX_FILES.uniq!

  LINUX_ARCHIVE_FILENAME = "public/gosu-linux-#{GOSU_VERSION}.tar.gz"

  desc "Build the archive #{LINUX_ARCHIVE_FILENAME}"
  task :archive => [:cpp_docs, :version] do
    files = LINUX_FILES
    sh "cd .. && tar -czf gosu/#{LINUX_ARCHIVE_FILENAME} #{files.map { |filename| "'gosu/#{filename}'" }.join(' ')}"
  end

  desc "Releases the archive #{LINUX_ARCHIVE_FILENAME} on GitHub"
  task :release => :archive do
    # ...gets a broken pipe error...
    #sh "rake/upload #{LINUX_ARCHIVE_FILENAME} 'Gosu #{GOSU_VERSION} source package for Linux (C++)'"
  end
  
  task :gem => [:version]
  
  LINUX_SPEC = Gem::Specification.new do |s|
    apply_gemspec_defaults s
    s.platform = 'ruby'
    s.files = COMMON_RUBY_FILES + FileList[ 'GosuImpl/**/*', 'linux/extconf.rb' ]
    s.extensions = [ 'linux/extconf.rb' ]
    s.require_path = 'lib'
    s.requirements = ['See http://code.google.com/p/gosu/wiki/GettingStartedOnLinux']
  end
  
  Rake::GemPackageTask.new(LINUX_SPEC) { |t| t.package_dir = 'public' }

  task :release_gem => :gem do
    sh "gem push public/gosu-#{GOSU_VERSION}.gem"
  end
  
  task :ruby do
    sh "cd linux && rm -f Makefile && ruby extconf.rb && make"
  end
end

