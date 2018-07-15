#!/bin/sh
GENET_VER=$(jq -r '.version' package.json)
cp -r debian/. out/.debian
sed -e "s/{{GENET_VERSION}}/$GENET_VER/g" debian/DEBIAN/control > out/.debian/DEBIAN/control
mkdir -p out/.debian/usr/share/icons/hicolor/256x256/apps
cp images/genet.png out/.debian/usr/share/icons/hicolor/256x256/apps
cp -r out/genet-linux-x64/. out/.debian/usr/share/genet
mv out/.debian/usr/share/genet/genet out/.debian/usr/share/genet/genet
chrpath -r /usr/share/genet out/.debian/usr/share/genet/genet
(cd out/.debian && fakeroot dpkg-deb --build . ../genet-linux-amd64.deb)
