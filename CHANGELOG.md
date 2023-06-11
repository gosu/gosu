# Changelog

## [1.4.6] - 2023-05-20
* When using SDL 2.0.12 or later, the LED indicators on gamepads will now be set to match the gamepad index that Gosu has allocated for them. (#639)
* Added some missing `#include` statements, thanks to @LittleLightLittleFire for the PRs. (#641, #642)
* Update all bundled dependencies. (#643)

## [1.4.5] - 2022-12-24
* Finish the fix for the Windows main loop from #634, a key part was missing. (#636)
* Update all bundled dependencies, thanks to @cyberarm for the automation. (#635)

## [1.4.4] - 2022-12-12
* Add clipboard support in the form of `Gosu.clipboard`/`Gosu.clipboard=`. (Thanks @cyberarm! #630)
* Fix a regression in the `Window.show` main loop on Windows that was introduced in 1.4.3. (#634)
* Fix a potential build/linker error on ARM macOS. (Thanks @jimtng! #627)

## [1.4.3] - 2022-03-27
* Update bundled dependencies: stb_image, stb_image_write, stb_truetype, stb_vorbis, utf8proc, mojoAL, SDL_sound.
* These updates fix MP3 playback and Gosu::Sample pitch changes. (#592, #623).
* Introduce optional `retro: true` option in `Font#initialize`, thanks to @cyberarm. (#479, #624)

## [1.4.1] - 2022-01-30
* Fix an audio bug when repeatedly playing a Gosu::Song. (#618 / #619)
* Fixed Window::tick never waiting for update_interval. (#620 / #621)

Both fixes once again contributed by @cyberarm.

## [1.4.0] - 2022-01-21
* Windows: Support for Ruby 3.1. (#611/#612)
* Windows: Update SDL to 2.0.20 (#610), also update SDL_sound on all platforms (#606).
* Replace OpenAL (usually oal-soft) with mojoAL. (#608)

All three improvements contributed by @cyberarm.

## [1.3.0] - 2022-01-02
* Add `Window#gain_focus` callback to mirror `Window#lose_focus`, which was previously undocumented and only implemented on iOS. (#600) Thanks @cyberarm!
* Fix Gosu::Window#tick not raising errors from callbacks. (#594) Thanks again, @cyberarm!
* C++: Refactor the Gosu::Color interface to be less verbose.

## [1.2.0] - 2021-03-18
* `Gosu.language()` is deprecated, please use `Gosu.user_languages()` instead. 
* macOS: Fixed loading of bold/italic system TTF fonts.

## [1.1.1] - 2021-02-13

* Improved accuracy of `Gosu.available_width` / `Gosu.available_height`. Thanks @cyberarm!
* Fixed a bug where macros (images returned by `Gosu.record`) were deformed when rendered with specific coordinates. Thanks @jsb!
* macOS: Improved support for ARM-based Macs.

## [1.1.0] - 2021-01-20

* `Gosu::Window` can now be created with `borderless: true` (`WF_BORDERLESS` in C++) to hide all window chrome. Thanks to @cyberarm for this contribution.
* `Gosu::Window#resizable=` and `Gosu::Window#borderless=` allow changing these properties later.
* Windows: Pin the main thread to the first processor core during `Window::show` to avoid timing glitches.

## [1.0.0] – 2020-12-29

* Breaking change: `Image.from_text`, `Font.draw_text` and `Font.text_width` have stopped parsing pseudo-HTML markup. Replace "text" with "markup" in each method name to get this functionality back.
* Breaking change: The alternative `gosu/zen` interface has been removed.
* Breaking change: `Gosu::Window::needs_cursor?` now returns true by default, not false.
* Support for Ruby 3.0. Gosu now requires Ruby 2.5 or later, and can only be used on Windows when the RubyInstaller DevKit has been installed.
* Support for analog gamepad sticks. Previously, all sticks and D-pads of gamepad 1 were merged into the same constants such as `GP_0_LEFT`. Now there is a `GP_0_DPAD_LEFT` button ID for the D-pad, and `GP_LEFT_STICK_X_AXIS` / `GP_RIGHT_STICK_X_AXIS` axis IDs that which can be used with `Gosu.axis()` to query a value between -1 and 1. Thanks to @cyberarm for this contribution. (#524, #540, #542, #543. #546)
* Replace all audio libraries by a statically bundled copy of SDL_sound. (#539) 
* Added button IDs: `KB_PRINT_SCREEN`, `KB_SCROLL_LOCK`, `KB_PAUSE`, `KB_CAPS_LOCK`. Thanks to @edwinacunav for this contribution. (#545)
* Some method arguments have been made optional again, for compatibility with very early Ruby/Gosu projects.
* C++: Rewrite CMake files for Linux and macOS (#535), clean up MSVC 2019 project.
* Adds a C API to use Gosu from other languages (using FFI or other mechanisms); this is a work in progress, but there are proof of concept bindings for Crystal and MRuby. Join the Discord server to learn more. Thanks to @cyberarm for this contribution.
* Update all stb libraries.
* Windows: Update the bundled versions of SDL 2 (2.0.14) and OpenAL soft (1.21.0).

## [0.15.2] – 2020-06-02

* Fix compilation errors with gcc 10.

## [0.15.1] – 2020-01-21

* Update utf8proc and all used stb libraries. (#520)
* Windows: Support for Ruby 2.7. (#520)
* Windows: Update bundled SDL version to 2.0.10 (#520)

## [0.15.0] – 2020-01-21

* Introduce the `resizable: true` argument in Window#initialize  to create resizable windows (thanks @cyberarm). (#437)
* Introduce `Image.from_blob(w, h, rgba)`, which makes it much easier to create images from binary data than having to emulate the `RMagick::Image` interface.
* Fix a crash in debug mode when passing certain strings to `Image.from_text`. (#509)
* Fix bugs in `Image#insert`. (#515)
* Introduce `Color#argb`/`Color#to_i`. (#476)
* Deprecate `Color#bgr`/`Color#abgr`.

## [0.14.5] – 2019-01-09

* Add KB_NUMPAD_DELETE constant, thanks @PeterCamilleri!
* Windows: Compatibility with Ruby 2.6.
* Windows: Update bundled SDL version to 2.0.9.

## [0.14.4] – 2018-10-09

* Add diagnostic logging when the `GOSU_DEBUG` environment variable is set.
* Linux: Improve font fallback logic, which used to be severely broken on Fedora-based distributions.

## [0.14.3] – 2018-10-04

* F11 can now also be used by default to toggle fullscreen mode (many browsers and video players support this shortcut).
* `Gosu.render` now supports the `retro: true` argument in an option hash.
* Update all stb libraries.
* Windows, Linux: MP3 files now work both in Songs and Samples.
* Linux: Improve both the default and fallback font logic; this should fix bold/italic text with the default font, and exceptions on some distributions.

## [0.14.0] – 2018-09-17

* `Gosu.render(width, height) { ... }` can render arbitrary Gosu code to a new texture and return it as a `Gosu::Image`. This method may not be supported on computers with outdated OpenGL implementations. Thanks for @Kjarrigan for getting this method off the ground.
* Text rendering has been completely rewritten to use stb_truetype for font rendering on *all* platforms. Gosu now renders text the same on all platforms (as long as the fonts are the same).
* Ruby: `Gosu::Font#initialize` now accepts style parameters: `bold: true, italic: false`. Unlike in earlier versions of Gosu, `Font` is not bold by default.
* Ruby: `Font#draw_markup`/`Font#draw_text`, `Font#draw_markup_rel`/`Font#draw_text_rel`, and `Font#markup_width`/`Font#text_width`, all parse markup for compatibility with older versions of Gosu. The existing methods will stop parsing markup in Gosu 1.0. If you want to keep using the markup feature, you should use the `markup` methods from now on.
* Ruby: `Image.from_markup` (new) and `Image.from_text` both parse markup for compatibility with older versions of Gosu. `Image.from_text` will stop parsing markup in Gosu 1.0.
* Ruby: `Image.from_markup`/`Image.from_text` now supports `alignment: :center`, `alignment: :right`, and the `spacing:` parameter even when there is no `width:` parameter.
* Ruby: Remove support for Ruby 1.8-2.2 on Windows. Ruby 1.9 through 2.2 are still supported on macOS and Linux.
* macOS: Compatibility with macOS 10.14 Mojave. (#458)
* macOS: Fix mouse position in multi-monitor setups. (SDL bug 4255)
* iOS: Support for `Gosu::TextInput` on iOS.

## [0.13.3] – 2018-03-08

* Updated all stb dependencies. (#428)
* Windows: Support for Ruby 2.5. (#427)
* Windows: Update bundled SDL to 2.0.8.

## [0.13.2] – 2018-01-16

* Fix `Gosu::KB_SLASH` constant, thanks to @mblumtritt for reporting this issue. (#426)
* macOS: Fix compilation on macOS High Sierra with Apple's built-in Ruby 2.3, thanks to @jiazhh0325 for reporting this issue.

## [0.13.1] – 2017-11-25

* Brings `draw_rect` in line with other `draw_` methods (available in C++, available on `Gosu::Window` in Ruby).
* `SampleInstance` is now called `Channel`, but the old name still works in Ruby.
* Update the stb_image dependency to fix crashes when loading 1-bit PNG files.
* Windows: Update bundled SDL to 2.0.7.
* macOS: Work around a compiler bug in Xcode 9+.
* C++: Image and Sample have a default constructor now that creates an "empty" object; minor interface changes.

## [0.12.1] – 2016-06-17

* Windows: Support for Ruby 2.4. (#407)
* Windows: Fix MP3 playback on 32-bit systems. (#405)

## [0.12.0] – 2017-04-23

* `Image#subimage` works for images larger than 1022x1022px. (#398, #400)
* Windows, Linux: Support for MP3 files; requires `libmpg123-dev` on Linux. (#397)
* C++: Use `std::function` parameters instead of begin/end function pairs (#399).
* Raspberry Pi: Use correct OpenGL version on Pi 2 & 3.
* Add `Window#drop(filename)` callback for when the user drops a file onto the window (#378/#382). Thanks @Kjarrigan!
* Fix an inconsistency in `Color#from_hsv`. (#359)