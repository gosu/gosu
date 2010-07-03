#/bin/bash

source "$HOME/.rvm/scripts/rvm"
export CFLAGS=$RVM_CFLAGS
rvm install $RVM_RUBY --force -C --enable-shared,--with-arch=$RVM_ARCH,--build=$RVM_BUILD
gem install $RVM_GEMS
