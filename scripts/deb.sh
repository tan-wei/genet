#!/bin/sh
DEPLUG_VER=$(jq -r '.version' package.json)
cp -r debian/. out/.debian
sed -e "s/{{DEPLUG_VERSION}}/$DEPLUG_VER/g" debian/DEBIAN/control > out/.debian/DEBIAN/control
chmod 755 out/.debian/DEBIAN/postinst
mkdir -p out/.debian/usr/share/icons/hicolor/256x256/apps
cp images/deplug.png out/.debian/usr/share/icons/hicolor/256x256/apps
cp -r out/Deplug-linux-x64/. out/.debian/usr/share/deplug
mv out/.debian/usr/share/deplug/Deplug out/.debian/usr/share/deplug/deplug
chrpath -r /usr/share/deplug out/.debian/usr/share/deplug/deplug
(cd out/.debian && fakeroot dpkg-deb --build . ../deplug-linux-amd64.deb)
