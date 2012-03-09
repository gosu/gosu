# This Gemfile supplements the Rakefile, not Gosu itself.
# These gems are necessary to update the Gosu website.
# (Especially rake loves to break out of nowhere)

source :rubygems

gem 'yard'
gem 'rake', '< 0.9'
gem 'mysql'
gem 'rmagick'
case RUBY_PLATFORM
when /darwin/
  gem 'rb-appscript'
end
