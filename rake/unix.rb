namespace :unix do
  UNIX_FILES = COMMON_CPP_FILES +
               FileList['Gosu/*.hpp', 'src/**/*.{c,cpp,h,hpp,m,mm}',
                 'cmake/CMakeLists.txt',
                 'cmake/*.cmake',
                 'cmake/*.in',
                 'cmake/*.sh',
                 'cmake/README.md',
                 'examples/**/CMakeLists.txt']
  UNIX_FILES.uniq!

  UNIX_ARCHIVE_FILENAME = "pkg/gosu-#{GOSU_VERSION}.tar.gz"

  desc "Build the archive #{UNIX_ARCHIVE_FILENAME}"
  task :archive => [:set_version] do
    files = UNIX_FILES
    tar(UNIX_ARCHIVE_FILENAME, files)
  end

  desc "Releases the archive #{UNIX_ARCHIVE_FILENAME}"
  task :release => :archive do
    sh "scp '#{UNIX_ARCHIVE_FILENAME}' #{ENV['PROJECTS_HOST']}:#{ENV['PROJECTS_ROOT']}/libgosu.org/downloads/"
  end
end
