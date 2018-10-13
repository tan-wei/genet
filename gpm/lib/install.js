const objpath = require('object-path')
const path = require('path')
const fs = require('fs-extra')
async function installPackage (source, pkgDir, dir) {
  const pkgjson = path.resolve(dir, 'package.json')
  const pkg = await fs.readJSON(pkgjson)
  const { name } = pkg
  const dst = path.join(pkgDir, name)
  if (await fs.exists(dst)) {
    await fs.remove(dst)
  }
  objpath.set(pkg, '_gpm.source', source)
  await fs.ensureDir(path.dirname(dst))
  await fs.copy(dir, dst)
  await fs.writeJSON(path.resolve(dst, 'package.json'), pkg)
}

module.exports = installPackage
