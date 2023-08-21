curl https://cache.ruby-lang.org/pub/ruby/1.8/ruby-1.8.7.tar.gz | tar -xz
cd ruby-1.8.7

# Can't be bothered to install openssl when it's not even used. Just remove it.
rm -rf ext/openssl

./configure
make
