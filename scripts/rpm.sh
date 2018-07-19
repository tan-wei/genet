#!/bin/sh

GENET_VER=$(jq -r '.version | gsub("-";"~")' package.json)
mkdir -p out/src/.rpm
(cd out/src/.rpm && mkdir -p SOURCES BUILD RPMS SRPMS)
sed -e "s/{{GENET_VERSION}}/$GENET_VER/g" genet.rpm.spec > out/src/.rpm/genet.rpm.spec
cp -r debian/usr out/src/.rpm/BUILD
mkdir -p out/src/.rpm/BUILD/usr/share/icons/hicolor/256x256/apps
cp images/genet.png out/src/.rpm/BUILD/usr/share/icons/hicolor/256x256/apps
cp -r out/dist/genet-linux-x64/. out/src/.rpm/BUILD/usr/share/genet
mv out/src/.rpm/BUILD/usr/share/genet/genet out/src/.rpm/BUILD/usr/share/genet/genet
chrpath -r /usr/share/genet out/src/.rpm/BUILD/usr/share/genet/genet
rpmbuild --define "_topdir $PWD/out/src/.rpm" -bb out/src/.rpm/genet.rpm.spec
mv out/src/.rpm/RPMS/*/*.rpm out/genet-linux-x64-v${GENET_VER}.rpm
