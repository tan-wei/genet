#!/bin/sh

DEPLUG_VER=$(jq -r '.version | gsub("-";"~")' package.json)
mkdir -p out/.rpm
(cd out/.rpm && mkdir -p SOURCES BUILD RPMS SRPMS)
sed -e "s/{{DEPLUG_VERSION}}/$DEPLUG_VER/g" deplug.rpm.spec > out/.rpm/deplug.rpm.spec
cp -r debian/usr out/.rpm/BUILD
mkdir -p out/.rpm/BUILD/usr/share/icons/hicolor/256x256/apps
cp images/deplug.png out/.rpm/BUILD/usr/share/icons/hicolor/256x256/apps
cp -r out/Genet-linux-x64/. out/.rpm/BUILD/usr/share/deplug
mv out/.rpm/BUILD/usr/share/deplug/Genet out/.rpm/BUILD/usr/share/deplug/deplug
chrpath -r /usr/share/deplug out/.rpm/BUILD/usr/share/deplug/deplug
rpmbuild --define "_topdir $PWD/out/.rpm" -bb out/.rpm/deplug.rpm.spec
mv out/.rpm/RPMS/*/*.rpm out/deplug-linux-amd64.rpm
