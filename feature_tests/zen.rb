$LOAD_PATH << '../lib'
require 'gosu'
require 'gosu/zen'
include Gosu

update do
  set :caption, rand(564).to_s
end
