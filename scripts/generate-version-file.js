const path = require('path')
const os = require('os')
const fs = require('fs-extra')
const mkpath = require('mkpath')

const userPath = path.resolve(os.homedir(), '.genet')
const versionFile = path.join(userPath, '.version')

mkpath.sync(userPath)
const genet = fs.readJsonSync(
  path.join(__dirname, '../package.json'))

fs.writeFileSync(versionFile, JSON.stringify({
  genet: genet.version,
  negatron: genet.devDependencies['@genet/negatron'],
  abi: process.versions.modules,
  resourcePath: path.resolve(__dirname, '..'),
}))
