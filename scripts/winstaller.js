#!/usr/bin/env node

const { readJSON } = require('fs-extra')
const winstaller = require('electron-winstaller')

async function run() {
	const pkg = await readJSON('./package.json')
	const out = `genet-${process.platform}-${process.arch}-v${pkg.version}.exe`
  winstaller.createWindowsInstaller({
    appDirectory: 'out/dist/genet-win32-x64',
    outputDirectory: 'out',
    authors: 'h2so5',
    iconUrl: 'https://raw.githubusercontent.com/genet-app/images/master/genet.ico',
    setupIcon: 'images/genet-drive.ico',
    loadingGif: 'images/genet-install.gif',
    noMsi: true,
    setupExe: out,
    exe: 'genet.exe'
  })
}

run()