#!/usr/bin/env node

const { negatronVersion } = require('./negatron')
const path = require('path')
const packager = require('electron-packager')

const options = {
  dir: process.cwd(),
  download: { mirror: 'https://cdn.deplug.net/electron/v' },
  asar: { unpackDir: '{node_modules/@deplug/osx-helper,node_modules/nan,package,deplug-modules/bin}' },
  icon: 'images/deplug',
  ignore: /^(core|plugkit|scripts|snippet|debian|images)/,
  electronVersion: negatronVersion,
  win32metadata: {
    CompanyName: 'Deplug',
    FileDescription: 'Next generation packet analyzer',
    OriginalFilename: 'Deplug.exe',
    ProductName: 'Deplug',
    InternalName: 'Deplug',
  },
  out: path.join(process.cwd(), 'out'),
  overwrite: true,
}
packager(options).catch((err) => {
  console.warn(err)
  process.exit(1)
})
