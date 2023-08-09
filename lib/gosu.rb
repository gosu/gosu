require "ffi"

if RUBY_PLATFORM =~ /mswin$|mingw|win32\-|\-win32/
  binary_path = File.expand_path("../../gosu/lib", __dir__)
  # 64-bit builds of Windows use "x64-mingw" as RUBY_PLATFORM
  binary_path += "64" if RUBY_PLATFORM =~ /^x64-/

  begin
    # Make DLLs available as shown here:
    # https://github.com/oneclick/rubyinstaller2/wiki/For-gem-developers
    require "ruby_installer"
    RubyInstaller::Runtime.add_dll_directory(binary_path)
  rescue LoadError
    # Add this gem to the PATH on Windows so that bundled DLLs can be found.
    # When running through Ocra on Windows, we need to be careful to preserve the ENV["PATH"]
    # encoding (see #385).
    path_encoding = ENV["PATH"].encoding
    ENV["PATH"] = "#{binary_path.encode(path_encoding)};#{ENV["PATH"]}"
  end

  # Add the correct lib directory for the current version of Ruby (major.minor).
  $LOAD_PATH.prepend File.join(binary_path, RUBY_VERSION[/^\d+.\d+/])
end

module Gosu
end

require_relative "gosu/ffi"
require_relative "gosu/constants"

require_relative "gosu/numeric"

# Individual classes need to be loaded after defining GosuFFI.check_last_error.

require_relative "gosu/gosu"

require_relative "gosu/channel"
require_relative "gosu/color"
require_relative "gosu/font"
require_relative "gosu/gl_tex_info"
require_relative "gosu/image"
require_relative "gosu/sample"
require_relative "gosu/song"
require_relative "gosu/text_input"
require_relative "gosu/window"

require_relative "gosu/compat"
