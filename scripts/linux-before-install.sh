export CC="gcc-8"
export CXX="g++-8"
export CPATH=$CPATH:$HOME/usr/include
export LIBRARY_PATH=$LIBRARY_PATH:$HOME/usr/lib
wget http://www.tcpdump.org/release/libpcap-1.7.4.tar.gz
tar xzf libpcap-1.7.4.tar.gz
(cd libpcap-1.7.4 && ./configure -q --prefix=$HOME/usr --enable-shared=no && make -j2 && make install)
