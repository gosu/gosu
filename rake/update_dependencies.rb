# Gems
require "excon" # http client
require "zip"   # rubyzip gem

# Standard libary
require "json"
require "fileutils"
require "stringio"

# Adapted from https://gist.github.com/cyberarm/6e05d4b7108dd8b4ccdfdd0a0ee30dd0

# Tested against SDL2 2.26.1
task :update_sdl do
  github_api = "https://api.github.com/repos/libsdl-org/SDL/releases"
  target_directory = File.expand_path("../dependencies/SDL", __dir__)
  x86_dll_directory = File.expand_path("../lib", __dir__)
  x64_dll_directory = File.expand_path("../lib64", __dir__)

  start_time = Time.now # FIXME: use Process monotonic time

  puts "Updating SDL..."
  puts "  Requesting SDL's github releases..."
  response = Excon.get(github_api)

  if response.status == 200
    array = JSON.parse(response.body, symbolize_names: true)
    selected_release = array.find do |release|
      !release[:draft] && !release[:prerelease] && Integer(release[:name].split(".")[1]).even?
    end

    # TODO: Report failure if no release is selected

    selected_assets = []

    selected_assets << selected_release[:assets].find do |asset|
      (asset[:name].start_with?("SDL2-devel-") && asset[:name].end_with?("-VC.zip"))
    end

    selected_assets << selected_release[:assets].find do |asset|
      (asset[:name].start_with?("SDL2-devel-") && asset[:name].end_with?("-mingw.zip"))
    end

    # TODO: Report failure if no asset is selected

    selected_assets.each_with_index do |asset, i|
      is_visual_c = i.zero?
      buffer = StringIO.new

      streamer = lambda do |chunk, remaining_bytes, total_bytes|
        buffer << chunk
      end

      puts "  Downloading #{asset[:name]}..."
      response = Excon.get(
        asset[:browser_download_url],
        response_block: streamer,
        middlewares: Excon.defaults[:middlewares] + [Excon::Middleware::RedirectFollower],
      )

      if response.status == 200

        # Remove SDL directory and stub /include and /lib directories
        if is_visual_c
          puts "  Removing #{target_directory} directory..."
          FileUtils.remove_dir(target_directory)

          puts "  Creating #{target_directory} with required subdirectories..."
          FileUtils.mkdir_p(target_directory)
          FileUtils.mkdir_p("#{target_directory}/include")
          FileUtils.mkdir_p("#{target_directory}/lib/x86")
          FileUtils.mkdir_p("#{target_directory}/lib/x64")
        end

        Zip::File.open_buffer(buffer) do |zip_file|
          if is_visual_c
            puts "    Updating headers..."
            zip_file.glob("*/include/**").each do |entry|
              next if File.basename(entry.name).end_with?(".h.orig")

              File.write("#{target_directory}/include/#{File.basename(entry.name)}", zip_file.read(entry))
            end

            puts "    Updating VC SDL2..."
            zip_file.glob("*/lib/x*/**").each do |entry|
              if entry.name.end_with?("/x86/SDL2.lib")
                File.open("#{target_directory}/lib/x86/#{File.basename(entry.name)}", "wb") { |f| f.write zip_file.read(entry) }
              elsif entry.name.end_with?("/x64/SDL2.lib")
                File.open("#{target_directory}/lib/x64/#{File.basename(entry.name)}", "wb") { |f| f.write zip_file.read(entry) }
              end

              if entry.name.end_with?("/x86/SDL2.dll")
                File.open("#{x86_dll_directory}/#{File.basename(entry.name)}", "wb") { |f| f.write zip_file.read(entry) }
              elsif entry.name.end_with?("/x64/SDL2.dll")
                File.open("#{x64_dll_directory}/#{File.basename(entry.name)}", "wb") { |f| f.write zip_file.read(entry) }
              end
            end
          else # mingw
            puts "    Updating mingw SDL2.dll.a..."
            zip_file.glob("*/i686*-mingw32/lib/libSDL2.dll.a").each do |entry|
              pp entry.name
              File.open("#{target_directory}/lib/x86/#{File.basename(entry.name)}", "wb") { |f| f.write zip_file.read(entry) }
            end

            zip_file.glob("*/x86_64*-mingw32/lib/libSDL2.dll.a").each do |entry|
              pp entry.name
              File.open("#{target_directory}/lib/x64/#{File.basename(entry.name)}", "wb") { |f| f.write zip_file.read(entry) }
            end
          end
        end
      end
    end

    puts "Updated SDL. Took: #{(Time.now - start_time).to_f.round(1)} seconds."
    puts
    puts
  else
    puts "Unexpected HTTP status code: #{response.status}"
    exit
  end
end

task :update_sdl_sound do
  github_zip_url = "https://github.com/icculus/SDL_sound/archive/refs/heads/main.zip"
  target_directory = File.expand_path("../dependencies/SDL_sound", __dir__)

  start_time = Time.now # FIXME: use Process monotonic time

  puts "Updating SDL_sound..."
  puts "  Requesting SDL_sound's github repo zip..."

  buffer = StringIO.new

  streamer = lambda do |chunk, remaining_bytes, total_bytes|
    buffer << chunk
  end

  puts "  Downloading..."
  response = Excon.get(
    github_zip_url,
    response_block: streamer,
    middlewares: Excon.defaults[:middlewares] + [Excon::Middleware::RedirectFollower],
  )

  if response.status == 200
    puts "  Removing #{target_directory} directory..."
    FileUtils.remove_dir(target_directory)

    puts "  Creating #{target_directory}..."
    FileUtils.mkdir_p(target_directory)
    FileUtils.mkdir_p("#{target_directory}/libmodplug/")

    Zip::File.open_buffer(buffer) do |zip_file|
      puts "    Updating files..."
      zip_file.glob("*/src/**/**").each do |entry|
        # Skip MIDI bits since we can't currently use them...
        next if entry.name.include?("midi.c") || entry.name.include?("/timidity/")
        # Exclude 'example' file
        next if File.basename(entry.name).downcase == "SDL_sound_skeleton.c".downcase
        # Only include implementation and header files
        next unless entry.name.end_with?(".c") || entry.name.end_with?(".h")
        next if entry.name_is_directory?

        if entry.name.include?("/libmodplug/")
          File.write("#{target_directory}/libmodplug/#{File.basename(entry.name)}", zip_file.read(entry))
        else
          File.write("#{target_directory}/#{File.basename(entry.name)}", zip_file.read(entry))
        end
      end
    end

    puts "", "  NOTE: You'll need to manually change #{target_directory}/SDL_sound_internal.h to disable MIDI support. 'SOUND_SUPPORTS_MIDI'", ""

    puts "Updated SDL_sound. Took: #{(Time.now - start_time).to_f.round(1)} seconds."
    puts
    puts
  else
    puts "Unexpected HTTP status code: #{response.status}"
    exit
  end
end

task :update_mojoal do
  github_zip_url = "https://github.com/icculus/mojoAL/archive/refs/heads/main.zip"
  target_directory = File.expand_path("../dependencies/mojoAL", __dir__)

  start_time = Time.now # FIXME: use Process monotonic time

  puts "Updating mojoAL..."
  puts "  Requesting mojoAL's github repo zip..."

  buffer = StringIO.new

  streamer = lambda do |chunk, remaining_bytes, total_bytes|
    buffer << chunk
  end

  puts "  Downloading..."
  response = Excon.get(
    github_zip_url,
    response_block: streamer,
    middlewares: Excon.defaults[:middlewares] + [Excon::Middleware::RedirectFollower],
  )

  if response.status == 200
    puts "  Removing #{target_directory} directory..."
    FileUtils.remove_dir(target_directory)

    puts "  Creating #{target_directory}..."
    FileUtils.mkdir_p(target_directory)
    FileUtils.mkdir_p("#{target_directory}/AL/")

    Zip::File.open_buffer(buffer) do |zip_file|
      puts "    Updating files..."
      zip_file.each do |entry|
        # Only include implementation and header files
        next unless entry.name.end_with?(".c") || entry.name.end_with?(".h")
        next if entry.name.include?("/tests/")
        next if entry.name_is_directory?

        if entry.name.include?("/AL/")
          File.write("#{target_directory}/AL/#{File.basename(entry.name)}", zip_file.read(entry))
        else
          File.write("#{target_directory}/#{File.basename(entry.name)}", zip_file.read(entry))
        end
      end
    end

    puts "", "  NOTE: You'll need to manually change #{target_directory}/mojoAL.c to include 'AL/*.h' instead of '*.h'", ""

    puts "Updated mojoAL. Took: #{(Time.now - start_time).to_f.round(1)} seconds."
    puts
    puts
  else
    puts "Unexpected HTTP status code: #{response.status}"
    exit
  end
end

task :update_stb do
  files = %w[ stb_image_write.h stb_image.h stb_truetype.h ]

  github_zip_url = "https://github.com/nothings/stb/archive/refs/heads/master.zip"
  target_directory = File.expand_path("../dependencies/stb", __dir__)

  start_time = Time.now # FIXME: use Process monotonic time

  puts "Updating stb..."
  puts "  Requesting stb's github repo zip..."

  buffer = StringIO.new

  streamer = lambda do |chunk, remaining_bytes, total_bytes|
    buffer << chunk
  end

  puts "  Downloading..."
  response = Excon.get(
    github_zip_url,
    response_block: streamer,
    middlewares: Excon.defaults[:middlewares] + [Excon::Middleware::RedirectFollower],
  )

  if response.status == 200
    puts "  Removing #{target_directory} directory..."
    FileUtils.remove_dir(target_directory)

    puts "  Creating #{target_directory}..."
    FileUtils.mkdir_p(target_directory)

    Zip::File.open_buffer(buffer) do |zip_file|
      puts "    Updating files..."
      zip_file.each do |entry|
        # Only include header files
        next unless entry.name.end_with?(".h")
        next if entry.name_is_directory?
        next unless files.include?(File.basename(entry.name))

        File.write("#{target_directory}/#{File.basename(entry.name)}", zip_file.read(entry))
      end
    end

    puts "Updated stb. Took: #{(Time.now - start_time).to_f.round(1)} seconds."
    puts
    puts
  else
    puts "Unexpected HTTP status code: #{response.status}"
    exit
  end
end

task :update_utf8proc do
  github_api = "https://api.github.com/repos/JuliaStrings/utf8proc/releases"
  target_directory = File.expand_path("../dependencies/utf8proc", __dir__)

  start_time = Time.now # FIXME: use Process monotonic time

  puts "Updating utf8proc..."
  puts "  Requesting utf8proc's github releases..."
  response = Excon.get(github_api)

  if response.status == 200
    array = JSON.parse(response.body, symbolize_names: true)
    selected_release = array.find do |release|
      !release[:draft] && !release[:prerelease] && Integer(release[:name].split(".")[1]).even?
    end

    # TODO: Report failure if no release is selected

    buffer = StringIO.new

    streamer = lambda do |chunk, remaining_bytes, total_bytes|
      buffer << chunk
    end

    puts "  Downloading..."
    response = Excon.get(
      selected_release[:zipball_url],
      response_block: streamer,
      middlewares: Excon.defaults[:middlewares] + [Excon::Middleware::RedirectFollower],
    )

    if response.status == 200
      puts "  Removing #{target_directory} directory..."
      FileUtils.remove_dir(target_directory)

      puts "  Creating #{target_directory}..."
      FileUtils.mkdir_p(target_directory)

      Zip::File.open_buffer(buffer) do |zip_file|
        zip_file.glob("*/*.{c,h}").each do |entry|
          # Rename utf8proc_data.c to utf8proc_data.h
          if File.basename(entry.name).downcase == "utf8proc_data.c".downcase
            File.write("#{target_directory}/#{File.basename(entry.name, ".c")}.h", zip_file.read(entry))
          else
            File.write("#{target_directory}/#{File.basename(entry.name)}", zip_file.read(entry))
          end
        end
      end
    end

    puts "", "  NOTE: You'll need to manually change #{target_directory}/utf8proc.c to include 'utf8proc_data.h' instead of 'utf8proc_data.c'", ""

    puts "Updated utf8proc. Took: #{(Time.now - start_time).to_f.round(1)} seconds."
    puts
    puts
  else
    puts "Unexpected HTTP status code: #{response.status}"
    exit
  end
end

task update_dependencies: [:update_sdl, :update_sdl_sound, :update_mojoal, :update_stb, :update_utf8proc]
