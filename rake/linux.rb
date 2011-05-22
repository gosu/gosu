namespace :linux do
  LINUX_FILES = COMMON_CPP_FILES + COMMON_RUBY_FILES + FileList[
    'Gosu', 'GosuImpl', 'linux/configure', 'linux/configure.ac', 'linux/Makefile.in'
  ]
  LINUX_FILES.uniq!

  LINUX_ARCHIVE_FILENAME = "public/gosu-linux-#{GOSU_VERSION}.tar.gz"

  desc "Build the archive #{LINUX_ARCHIVE_FILENAME}"
  task :archive => [:cpp_docs, :version] do
    files = LINUX_FILES
    Dir.chdir('..') do
      tar("gosu/#{LINUX_ARCHIVE_FILENAME}", files.map { |fn| "gosu/#{fn}" })
    end
  end

  desc "Releases the archive #{LINUX_ARCHIVE_FILENAME} on GitHub"
  task :release => :archive do
    upload LINUX_ARCHIVE_FILENAME
  end
  
  task :gem => [:version]
  
  LINUX_SPEC = Gem::Specification.new do |s|
    apply_gemspec_defaults s
    s.platform = 'ruby'
    s.files = COMMON_RUBY_FILES + FileList[ 'GosuImpl/**/*', 'linux/extconf.rb' ]
    s.extensions = [ 'linux/extconf.rb' ]
    s.require_path = 'lib'
    s.requirements = ['See https://github.com/jlnr/gosu/wiki/Getting-Started-on-Linux']
  end
  
  Rake::GemPackageTask.new(LINUX_SPEC) { |t| t.package_dir = 'public' }

  task :release_gem => :gem do
    sh "gem push public/gosu-#{GOSU_VERSION}.gem"
  end
  
  task :ruby do
    sh "cd linux && rm -f Makefile && ruby extconf.rb && make"
  end
end

