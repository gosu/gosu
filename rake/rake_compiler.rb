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
  s.required_ruby_version = Gem::Requirement.new(">= 1.9.3")
  s.platform = Gem::Platform::RUBY
  s.extensions = FileList["ext/**/extconf.rb"]
  # Ruby frontend for C++ extension.
  s.files += Dir["lib/**/*.rb"]
  # Gosu dependencies
  s.files += Dir["dependencies/{SDL_sound,stb,utf8proc}/**/*.{h,c}"]
  # Gosu source code including the Ruby interface (.cxx).
  s.files += Dir["src/**/*.{h,hpp,c,cpp,cxx}"]
  # Gosu header files, for compiling the gem & for using inline C++ from Ruby.
  s.files += Dir["include/**/*.{h,hpp}"]
  # RDoc setup:
  # The docs will never look great in rdoc, though, because we are using some yard-specific syntax
  # in rdoc/gosu.rb.
  # To generate more useful documentation, run "yard" in the git root.
  s.files += Dir["README.md", "COPYING", "rdoc/gosu.rb", ".yardopts"]
  s.rdoc_options += %w(-m README.md -x lib)
  s.extra_rdoc_files = %w(README.md COPYING rdoc/gosu.rb)
end

Gem::PackageTask.new(GEM_SPEC) do |pkg|
end

Rake::ExtensionTask.new("gosu", GEM_SPEC)
