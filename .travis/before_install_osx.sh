#!/bin/bash
set -ev


# Mac dependencies
# See: https://github.com/gosu/gosu/wiki/Getting-Started-on-OS-X
brew update
brew install sdl2


# Update the system RubyGems on OS X and install CocoaPods.
# Also install a very specific version of ActiveSupport, see:
# https://github.com/CocoaPods/CocoaPods/issues/4711#issuecomment-230096751
# Also install gem binaries into /usr/local/bin to work around SIP on macOS 10.11+
sudo gem update --system
sudo gem install activesupport -v 4.2.6
sudo gem install cocoapods -n /usr/local/bin
