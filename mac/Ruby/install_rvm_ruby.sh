#/bin/bash

source "$HOME/.rvm/scripts/rvm"
(rvm remove --gems $RVM_RUBY) || true
export CFLAGS=$RVM_CFLAGS
rvm install $RVM_RUBY -C --enable-shared,--with-arch=$RVM_ARCH,--build=$RVM_BUILD
rvm use $RVM_RUBY
gem install $RVM_GEMS
