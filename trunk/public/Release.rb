require 'ftools'

class Package
  def initialize(name)
    @name = name
  end
  def add_file(file)
    dest = "#{@name}/#{file}"
    File.makedirs(File.dirname(dest))
    File.copy("../#{file}", dest)
  end
  def add_link(link)
    dest = "#{@name}/#{link}"
    File.makedirs(File.dirname(dest))
    File.symlink(File.readlink("../#{link}"), dest)
  end
  def add_dir(dir)
    Dir.foreach("../#{dir}") do |item|
      case File.ftype("../#{dir}/#{item}")
      when "file" then
        add_file("#{dir}/#{item}")
      when "link" then
        add_link("#{dir}/#{item}")
      when "directory" then
        add_dir("#{dir}/#{item}") unless item[0, 1] == "."
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

if ARGV[0] == 'source' then
  source = Package.new("gosu-source-#{ARGV[1]}")
  source.add_file("COPYING.txt")
  source.add_file("README.txt")
  source.add_dir("Gosu")
  source.add_dir("GosuImpl")
  source.add_dir("reference")
  source.add_dir("examples")
  source.add_dir("linux")
  source.add_file("mac/Gosu.xcodeproj/project.pbxproj")
  source.add_file("mac/English.lproj/InfoPlist.strings")
  source.add_file("mac/Gosu-Info.plist")
  source.add_file("mac/RubyGosu Template-Info.plist")
  source.add_file("mac/Gosu.icns")
  source.add_file("windows/Gosu.sln")
  source.add_file("windows/Gosu.vcproj")
  source.add_file("windows/RubyGosu.vcproj")
  source.targz_and_kill
  `./googlecode_upload.py --summary="Gosu #{ARGV[1]} source package (to compile on Linux)" --project=gosu --user=julianraschke --labels="Featured,Type-Source,OpSys-All" gosu-source-#{ARGV[1]}.tar.gz`
end

if ARGV[0] == 'mac' then
  mac = Package.new("gosu-mac-#{ARGV[1]}")
  mac.add_file("COPYING.txt")
  mac.add_file("README.txt")
  mac.add_dir("Gosu.framework")
  mac.add_file("gosu.bundle")
  mac.add_dir("RubyGosu Deployment Template.app")
  mac.add_dir("reference")
  mac.add_dir("examples")
  mac.targz_and_kill
  `./googlecode_upload.py --summary="Gosu #{ARGV[1]} precompiled for Mac OS X (C++ & Ruby)" --project=gosu --user=julianraschke --labels="Featured,Type-Archive,OpSys-OSX" gosu-mac-#{ARGV[1]}.tar.gz`
end
