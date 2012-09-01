namespace :linux do
  LINUX_FILES = COMMON_CPP_FILES +
                COMMON_RUBY_FILES +
                FileList['Gosu', 'GosuImpl',
                  'cmake/CMakeLists.txt',
                  'cmake/GosuConfig.cmake.in',
                  'cmake/gosu.pc.in',
                  'cmake/CMakeLists.txt.in',
                  'cmake/build.sh',
                  'cmake/create_deb_package.sh',
                  'cmake/create_tgz_package.sh',
                  'cmake/Packaging.cmake',
                  'cmake/GosuImpl.cmake',
                  ]
  LINUX_FILES.uniq!

  LINUX_ARCHIVE_FILENAME = "pkg/gosu-linux-#{GOSU_VERSION}.tar.gz"

  desc "Build the archive #{LINUX_ARCHIVE_FILENAME}"
  task :archive => [:cpp_docs, :set_version] do
    files = LINUX_FILES
    Dir.chdir('..') do
      tar("gosu/#{LINUX_ARCHIVE_FILENAME}", files.map { |fn| "gosu/#{fn}" })
    end
  end

  desc "Releases the archive #{LINUX_ARCHIVE_FILENAME}"
  task :release => :archive do
    upload LINUX_ARCHIVE_FILENAME
  end
  
  task :gem => :set_version
  
  LINUX_SPEC = Gem::Specification.new do |s|
    apply_gemspec_defaults s
    s.platform = 'ruby'
    s.files = COMMON_RUBY_FILES
    s.files += FileList['GosuImpl/**/*', 'linux/extconf.rb']
    s.extensions = ['linux/extconf.rb']
    s.requirements = ['See https://github.com/jlnr/gosu/wiki/Getting-Started-on-Linux']
  end
  
  Gem::PackageTask.new(LINUX_SPEC).define

  task :release_gem => :gem do
    sh "gem push pkg/gosu-#{GOSU_VERSION}.gem"
  end
  
  task :ruby do
    sh "cd linux && rm -f Makefile && ruby extconf.rb && make"
  end
end
