const path = require('path')
const os = require('os')
const fs = require('fs-extra')

const versionFile = process.env.GENET_VERSION_FILE ||
  path.join(os.homedir(), '.genet', '.version')

let resourcePath = null
let genetVersion = null
let genetAbiVersion = null
try {
  const ver = fs.readJsonSync(versionFile)
  resourcePath = ver
    .resourcePath
    .replace(/\bapp\.asar\b/, 'app.asar.unpacked')
  genetVersion = ver.genet
  genetAbiVersion = ver.abi
} catch (err) {
  resourcePath = null
  genetVersion = null
  genetAbiVersion = null
}

const builtinPackagePath = resourcePath
  ? path.join(resourcePath, 'package')
  : null
module.exports = {
  version: fs.readJSONSync(path.join(__dirname, '../package.json')).version,
  genetVersion,
  genetAbiVersion,
  genetTarget: process.env.GENET_TARGET || 'release',
  resourcePath,
  builtinPackagePath,
  userPackagePath: path.join(os.homedir(), '.genet', 'package'),
}
