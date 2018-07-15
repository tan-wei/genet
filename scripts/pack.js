#!/usr/bin/env node

const { negatronVersion } = require('./negatron')
const path = require('path')
const packager = require('electron-packager')

const options = {
  dir: process.cwd(),
  download: { mirror: 'https://cdn.deplug.net/electron/v' },
  asar: { unpackDir: '{node_modules/@deplug/osx-helper,node_modules/nan,package,genet-modules/bin}' },
  icon: 'images/genet',
  ignore: /^(src|lib|scripts|debian|images)/,
  electronVersion: negatronVersion,
  win32metadata: {
    CompanyName: 'Genet',
    FileDescription: 'General Purpose Network Analyzer',
    OriginalFilename: 'Genet.exe',
    ProductName: 'Genet',
    InternalName: 'Genet',
  },
  out: path.join(process.cwd(), 'out'),
  overwrite: true,
}
packager(options).catch((err) => {
  console.warn(err)
  process.exit(1)
})
