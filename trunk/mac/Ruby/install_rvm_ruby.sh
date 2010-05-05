#/bin/bash

source "$HOME/.rvm/scripts/rvm"
export CFLAGS=$RVM_CFLAGS
rvm install $RVM_RUBY --force -C --enable-shared,--with-arch=$RVM_ARCH,--build=$RVM_BUILD
# Gems somehow broken for 1.9.2 preview 1.
# Otherwise we should be able to build those.
# gem install chipmunk ruby-opengl eventmachine rev
