const glob = require('glob')
const path = require('path')
const objpath = require('object-path')
const fs = require('fs-extra')
const flatten = require('lodash.flatten')
const Package = require('./pkg')
async function listPackages (option = {}) {
  const { builtinDirs, userDirs } = Object.assign({
    builtinDirs: [],
    userDirs: [],
  }, option)

  const globOptions = {
    ignore: '**/node_modules/*/package.json',
    absolute: true,
    nodir: true,
  }
  const buitinPkgs = await Promise.all(flatten(builtinDirs.map((dir) =>
    glob.sync(path.join(dir, '/*/package.json'), globOptions)))
    .map((pkgJson) => fs.readJson(pkgJson)
      .then((meta) =>
        new Package(path.dirname(pkgJson), meta, ':builtin:'))))

  const userPkgs = await Promise.all(flatten(userDirs.map((dir) =>
    glob.sync(path.join(dir, '/**/package.json'), globOptions)))
    .map((pkgJson) => fs.readJson(pkgJson)
      .then((meta) => new Package(path.dirname(pkgJson), meta,
        objpath.get(meta, '_gpm.source', '')))))

  const merged = [].concat(userPkgs)
  for (const pkg of buitinPkgs) {
    const { metadata: name } = pkg
    if (!merged.find((item) => item.metadata.name === name)) {
      merged.push(pkg)
    }
  }
  return buitinPkgs.concat(userPkgs)
}

module.exports = listPackages
