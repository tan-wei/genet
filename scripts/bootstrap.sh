#!/bin/sh

mkdir -p /deplug/tools/bin
unzip -q -o /root/.electron/electron-*.zip -d /deplug/tools/electron

case $ELECTRON_PLATFORM in
  darwin ) ln -f -s ../electron/Electron.app/Contents/MacOS/Electron /deplug/tools/bin/electron ;;
  linux  ) ln -f -s ../electron/electron /deplug/tools/bin/electron ;;
  win32  ) echo "Please run 'MKLINK tools/bin/electron ../electron/Electron.exe'" ;;
  *      ) echo "unknown platform";;
esac
