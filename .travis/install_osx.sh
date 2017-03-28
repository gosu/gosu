#!/bin/bash
set -ev

# Gosu for OS X using CocoaPods
cd examples/Tutorial
pod install
xcodebuild build -workspace Tutorial.xcworkspace -scheme Tutorial
cd ../..

# Gosu for iOS using CocoaPods
cd examples/Tutorial-Touch
pod install
xcodebuild build -workspace Tutorial-Touch.xcworkspace -scheme Tutorial-Touch -destination 'platform=iOS Simulator,name=iPad Air'
cd ../..
