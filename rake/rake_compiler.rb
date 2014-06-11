GEM_SPEC = Gem::Specification.new do |s|
  s.name = 'gosu'
  s.version = GOSU_VERSION.dup
  s.summary = '2D game development library.'
  s.description = <<EOS
    2D game development library.

    Gosu features easy to use and game-friendly interfaces to 2D graphics
    and text (accelerated by 3D hardware), sound samples and music as well as
    keyboard, mouse and gamepad/joystick input.

    Also includes demos for integration with RMagick, Chipmunk and OpenGL.
EOS
  s.author = 'Julian Raschke'
  s.date = Time.now.strftime '%Y-%m-%d'
  s.email = 'julian@raschke.de'
  s.homepage = 'http://www.libgosu.org/'
  s.required_ruby_version = Gem::Requirement.new('>= 1.8.2')
  s.platform = Gem::Platform::RUBY
  s.extensions = FileList["ext/**/extconf.rb"]
  # Ruby frontend for C++ extension
  s.files += Dir["lib/**/*.rb"]
  # SWIG-generated Ruby wrapper
  s.files += Dir["ext/gosu/gosu_wrap.{h,cxx}"]
  # C++ Gosu source code
  s.files += Dir["src/**/*.{h,hpp,c,cpp,m,mm}"]
  # C++ Gosu header files, for compiling the gem & for using inline C++ from Ruby
  s.files += Dir["Gosu/*.hpp"]
  # Examples
  s.files += Dir["examples/*.rb"] + Dir["examples/media/**/*"]
end

Gem::PackageTask.new(GEM_SPEC) do |pkg|
end

Rake::ExtensionTask.new('gosu', GEM_SPEC)
