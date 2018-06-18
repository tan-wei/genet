#!/bin/sh
DEPLUG_VER=$(jq -r '.version' package.json)
cp -r debian/. out/.debian
sed -e "s/{{GENET_VERSION}}/$DEPLUG_VER/g" debian/DEBIAN/control > out/.debian/DEBIAN/control
chmod 755 out/.debian/DEBIAN/postinst
mkdir -p out/.debian/usr/share/icons/hicolor/256x256/apps
cp images/deplug.png out/.debian/usr/share/icons/hicolor/256x256/apps
cp -r out/Genet-linux-x64/. out/.debian/usr/share/genet
mv out/.debian/usr/share/genet/Genet out/.debian/usr/share/genet/genet
chrpath -r /usr/share/genet out/.debian/usr/share/genet/genet
(cd out/.debian && fakeroot dpkg-deb --build . ../genet-linux-amd64.deb)
