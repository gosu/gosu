# Replace the load path
$LOAD_PATH.clear
$LOAD_PATH << File.dirname(__FILE__)[0..-6]
$LOAD_PATH << $LOAD_PATH[0] + '/lib'
# Ruby portions of Gosu
require 'gosu/patches'
require 'gosu/swig_patches'
# Let the application know it is being run from the Mac app wrapper.
OSX_EXECUTABLE = true
# Main application
require 'Main'
