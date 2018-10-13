const path = require('path')
const objpath = require('object-path')
const fs = require('fs-extra')
const semver = require('semver')
async function verifyPackage (dir, version) {
  const pkgjson = path.resolve(dir, 'package.json')
  const pkg = await fs.readJSON(pkgjson)
  const engine = objpath.get(pkg, 'engines.genet', null)
  if (engine === null) {
    throw new Error('engines.genet field required in package.json')
  }
  if (!semver.satisfies(semver.coerce(version), engine)) {
    throw new Error(
      `genet@${version} does not satisfy required range: ${engine}`)
  }
}

module.exports = verifyPackage
