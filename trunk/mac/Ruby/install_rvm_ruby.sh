#/bin/bash

source "$HOME/.rvm/scripts/rvm"
export CFLAGS=$RVM_CFLAGS $CFLAGS
export LDFLAGS="$RVM_LDFLAGS -lobjc $LDFLAGS"
export DLDFLAGS="$RVM_LDFLAGS -lobjc $DLDFLAGS"
export XLDFLAGS="$RVM_LDFLAGS -lobjc $XLDFLAGS"
rvm install $RVM_RUBY --force -C --enable-shared,--with-arch=$RVM_ARCH,--build=$RVM_BUILD &&
rvm use $RVM_RUBY &&
gem install $RVM_GEMS
