#/bin/bash

source "$HOME/.rvm/scripts/rvm"
export CFLAGS=$RVM_CFLAGS
# Does crash because Rubygems lets Ruby 1.9.2 spill compiler guts everywhere at least on PPC
# Hence, || true
rvm install $RVM_RUBY --force -C --enable-shared,--with-arch=$RVM_ARCH,--build=$RVM_BUILD || true
# Gems somehow broken for 1.9.2 preview 1.
# Otherwise we should be able to build those.
# gem install chipmunk ruby-opengl eventmachine rev
