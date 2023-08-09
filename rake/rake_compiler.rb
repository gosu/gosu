GEM_SPEC = Gem::Specification.new do |s|
  s.name = "gosu"
  s.version = GOSU_VERSION.dup
  s.license = "MIT"
  s.summary = "2D game development library."
  s.description = <<EOS
    2D game development library.

    Gosu provides simple and game-friendly interfaces to 2D graphics
    and text (accelerated by 3D hardware), sound samples and music as well as
    keyboard, mouse and gamepad/joystick input.
EOS
  s.author = "Julian Raschke"
  s.date = Time.now.strftime "%Y-%m-%d"
  s.email = "julian@raschke.de"
  s.homepage = "https://www.libgosu.org/"
  s.required_ruby_version = Gem::Requirement.new(">= 2.5.0")
  s.platform = Gem::Platform::RUBY
  s.extensions = FileList["ext/**/extconf.rb"]
  # Ruby source files plus DLLs (for Windows).
  s.files += Dir["{lib,lib64}/**/*.{rb,dll}"]
  # Gosu dependencies.
  s.files += Dir["dependencies/**/*.{h,c,a}"]
  s.add_dependency "ffi", "~> 1.12"
  # Gosu source code including the Ruby interface (.cxx).
  s.files += Dir["{src,include,ffi}/**/*.{h,hpp,c,cpp}"] - ["src/WinMain.cpp"]
  # RDoc/Yard setup.
  s.files += Dir["README.md", "COPYING", "rdoc/gosu.rb", ".yardopts"]
  s.rdoc_options += %w(-m README.md -x lib)
  s.extra_rdoc_files = %w(README.md COPYING rdoc/gosu.rb)
end

Gem::PackageTask.new(GEM_SPEC).define

Rake::ExtensionTask.new("gosu-ffi", GEM_SPEC)
