#!/usr/bin/env node

const { negatronVersion } = require('./negatron')
const path = require('path')
const packager = require('electron-packager')

const options = {
  dir: process.cwd(),
  download: { mirror: 'https://storage.googleapis.com/cdn-genet-app/electron/v' },
  asar: { unpackDir: '{node_modules/@deplug/osx-helper,node_modules/nan,package,genet-modules/bin}' },
  icon: 'images/genet',
  ignore: /^(src|lib|scripts|debian|images)/,
  electronVersion: negatronVersion,
  win32metadata: {
    CompanyName: 'genet',
    FileDescription: 'General Purpose Network Analyzer',
    OriginalFilename: 'genet.exe',
    ProductName: 'genet',
    InternalName: 'genet',
  },
  out: path.join(process.cwd(), 'out'),
  overwrite: true,
}
packager(options).catch((err) => {
  console.warn(err)
  process.exit(1)
})
