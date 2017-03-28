#!/bin/bash

mkdir -p /deplug/tools
unzip -q -o /root/.electron/electron-*.zip -d /deplug/tools/electron

if [ "$ELECTRON_PLATFORM" == "darwin" ]; then
  ln -f -s ../electron/Electron.app/Contents/MacOS/Electron /deplug/tools/electron/electron
fi
