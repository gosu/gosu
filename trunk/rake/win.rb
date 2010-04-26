namespace :win do
  WINDOWS_CPP_ARCHIVE_FILENAME = "public/gosu-windows-cpp-#{GOSU_VERSION}.zip"
  
  desc "Build the archive #{WINDOWS_CPP_ARCHIVE_FILENAME}"
  task :cpp_archive => [:version] do
    files = COMMON_CPP_FILES + FileList['Gosu/*.hpp', 'lib/Gosu.lib', 'lib/GosuDebug.lib', 'lib/fmod.dll']
    sh "zip #{WINDOWS_CPP_ARCHIVE_FILENAME} #{files.map { |filename| "'#{filename}'" }.join(' ')}"
  end

  desc "Releases the archive #{WINDOWS_CPP_ARCHIVE_FILENAME} on GoogleCode"
  task :cpp_release => :cpp_archive do
    sh "./googlecode_upload.py --summary=\"Gosu #{GOSU_VERSION}, compiled for 32-bit Windows (C++)\"" +
       " --project=gosu --user=julianraschke --labels=\"Featured,Type-Archive,OpSys-Windows\" #{WINDOWS_CPP_ARCHIVE_FILENAME}"
  end

  WINDOWS_RUBY_ARCHIVE_FILENAME = "public/gosu-windows-ruby-#{GOSU_VERSION}.zip"

  desc "Build the archive #{WINDOWS_RUBY_ARCHIVE_FILENAME}"
  task :ruby_archive => [:version] do
    files = COMMON_RUBY_FILES + ['lib/gosu.for_1_8.so', 'lib/gosu.for_1_9.so', 'lib/fmod.dll']
    sh "zip #{WINDOWS_RUBY_ARCHIVE_FILENAME} #{files.map { |filename| "'#{filename}'" }.join(' ')}"
  end

  desc "Releases the archive #{WINDOWS_RUBY_ARCHIVE_FILENAME} on GoogleCode"
  task :ruby_release => :ruby_archive do
    sh "./googlecode_upload.py --summary=\"Gosu #{GOSU_VERSION}, compiled for 32-bit Windows (Ruby)\"" +
       " --project=gosu --user=julianraschke --labels=\"Featured,Type-Archive,OpSys-Windows\" #{WINDOWS_RUBY_ARCHIVE_FILENAME}"
  end
  
  desc "Release both #{WINDOWS_CPP_ARCHIVE_FILENAME} and #{WINDOWS_RUBY_ARCHIVE_FILENAME}"
  task :release => [:cpp_release, :ruby_release]

  task :gem => [:version]

  WINDOWS_SPEC = Gem::Specification.new do |s|
    apply_gemspec_defaults s
    s.platform = 'i386-mingw32'#'x86-mswin32-60' ...you can always force the gem platform
    s.files = COMMON_RUBY_FILES + ['lib/gosu.for_1_8.so', 'lib/gosu.for_1_9.so', 'lib/fmod.dll']
  end
  Rake::GemPackageTask.new(WINDOWS_SPEC) { |t| t.package_dir = 'public/windows_gem' }
  
  task :release_gem => :gem do
    sh "gem push public/windows_gem/gosu-#{GOSU_VERSION}-x86-mingw32.gem"
  end
end
