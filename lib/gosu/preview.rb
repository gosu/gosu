warn "gosu/preview.rb has been removed in Gosu 0.9.0, and Gosu itself \n" +
  "provides a similar interface to what preview.rb used to offer.\n" +
  "Notable differences:\n" +
  "• no global $window variable\n" +
  "• no global Gosu.mouse_x and Gosu.mouse_y functions\n" +
  "• Image#initialize et.al. use an options hash now";

require 'gosu'
