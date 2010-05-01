# Used by the RubyGosu App.app deployment package.

$LOAD_PATH.clear
$LOAD_PATH << File.dirname(__FILE__)[0..-6]
require 'gosu/patches'
require 'gosu/swig_patches'
require 'Main'
