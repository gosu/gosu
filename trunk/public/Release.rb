require 'fileutils'

class Package
  def initialize(name)
    @name = name
  end
  def add_file(file)
    dest = "#{@name}/#{file}"
    FileUtils.makedirs(File.dirname(dest))
    FileUtils.copy("../#{file}", "#{@name}/#{file}")
  end
  def add_dir(dir)
    Dir.foreach("../#{dir}") do |item|
      ftype = File.ftype("../#{dir}/#{item}")
      if ftype == "file" then add_file("#{dir}/#{item}") end
      if ftype == "directory" and not item[0, 1] == "." then
        add_dir("#{dir}/#{item}")
      end
    end
  end
  def del_file(file)
    File.delete("#{@name}/#{file}")
  end
  def targz_and_kill
    `tar -czf #{@name}.tar.gz #{@name}/; rm -rf #{@name}`
  end
  def zip_and_kill
    `cd #{@name}; zip -r ../#{@name}.zip *; cd ..; rm -rf #{@name}`
  end
end

source = Package.new("gosu-source-#{ARGV[0]}")
source.add_file("COPYING.txt")
source.add_file("index.html")
source.add_dir("Gosu")
source.add_dir("GosuImpl")
source.add_dir("RubyGosu")
source.add_dir("docs")
source.add_dir("examples")
source.add_dir("gcc")
source.add_file("mac/Gosu.xcodeproj/project.pbxproj")
source.add_file("mac/Game.xcodeproj/project.pbxproj")
source.add_dir("mac/GosuRes")
source.add_dir("mac/GameRes")
source.add_file("msvc/Gosu.sln")
source.add_file("msvc/Gosu.vcproj")
source.add_file("msvc/WinMain.vcproj")
source.add_file("msvc/RubyGosu.vcproj")
source.add_file("msvc/libpng.vcproj")
source.add_file("msvc/zlib.vcproj")
source.targz_and_kill

msvc71 = Package.new("gosu-msvc71-#{ARGV[0]}")
msvc71.add_file("COPYING.txt")
msvc71.add_file("index.html")
msvc71.add_dir("Gosu")
msvc71.add_dir("docs")
msvc71.add_dir("examples")
msvc71.add_file("lib/Gosu.lib")
msvc71.add_file("lib/GosuDyn.lib")
msvc71.add_file("lib/GosuDebug.lib")
msvc71.add_file("lib/GosuDebugDyn.lib")
msvc71.add_file("lib/WinMain.lib")
msvc71.add_file("lib/WinMainDyn.lib")
msvc71.add_file("lib/WinMainDebug.lib")
msvc71.add_file("lib/WinMainDebugDyn.lib")
msvc71.del_file("examples/Tutorial.rb")
msvc71.zip_and_kill

mac = Package.new("gosu-mac-#{ARGV[0]}")
mac.add_file("COPYING.txt")
mac.add_file("index.html")
mac.add_dir("docs")
mac.add_dir("examples")
mac.add_dir("mac/Gosu.framework")
mac.add_dir("mac/Game.app")
mac.zip_and_kill

ruby = Package.new("ruby-gosu-win32-#{ARGV[0]}")
ruby.add_file("COPYING.txt")
ruby.add_file("index.html")
ruby.add_dir("docs")
ruby.add_file("lib/gosu.so")
ruby.add_file("examples/Tutorial.rb")
ruby.add_dir("examples/media")
ruby.zip_and_kill
