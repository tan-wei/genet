const path = require('path')
const os = require('os')
const fs = require('fs')
const jsonfile = require('jsonfile')
const mkpath = require('mkpath')

const userPath = path.resolve(os.homedir(), '.deplug')
const versionFile = path.join(userPath, '.version')

mkpath.sync(userPath)
const deplug = jsonfile.readFileSync(
  path.join(__dirname, '../package.json'))

fs.writeFileSync(versionFile, JSON.stringify({
  deplug: deplug.version,
  negatron: deplug.devDependencies.negatron,
  abi: process.versions.modules,
  resourcePath: path.resolve(__dirname, '..'),
}))
