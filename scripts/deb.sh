#!/bin/sh
GENET_VER=$(jq -r '.version' package.json)
cp -r debian/. out/src/.debian
sed -e "s/{{GENET_VERSION}}/$GENET_VER/g" debian/DEBIAN/control > out/src/.debian/DEBIAN/control
mkdir -p out/src/.debian/usr/share/icons/hicolor/256x256/apps
cp images/genet.png out/src/.debian/usr/share/icons/hicolor/256x256/apps
cp -r out/dist/genet-linux-x64/. out/src/.debian/usr/share/genet
mv out/src/.debian/usr/share/genet/genet out/src/.debian/usr/share/genet/genet
chrpath -r /usr/share/genet out/src/.debian/usr/share/genet/genet
(cd out/src/.debian && fakeroot dpkg-deb --build . ../../genet-linux-amd64.deb)
