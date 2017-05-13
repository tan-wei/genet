export CC="gcc-5"
export CXX="g++-5"
wget http://www.tcpdump.org/release/libpcap-1.7.4.tar.gz
tar xzf libpcap-1.7.4.tar.gz
(cd libpcap-1.7.4 && ./configure -q --enable-shared=no && make -j2 && sudo make install)

export PATH=/home/travis/.gem/ruby/2.2.0/bin:$PATH
gem install --user-install deb-s3
