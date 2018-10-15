const crypto = require('crypto')
const glob = require('glob')
const path = require('path')
const semver = require('semver')
const getAbiVersion = require('./abi')
const { genetTarget } = require('./env')
class Package {
  constructor (dir, pkgJson, source) {
    Reflect.set(this, 'metadata', pkgJson)
    Reflect.set(this, 'source', source)
    Reflect.set(this, 'dir', dir)

    const abiVers = glob.sync(
      path.join(dir,
        `/target/${genetTarget}/*.{dll,so,dylib}`), { absolute: true })
      .map((file) => getAbiVersion(file))
      .filter((ver) => ver !== null)
    const sorted = semver.sort(abiVers.map((ver) => `${ver}.0`))
      .map((ver) => ver.split('.').slice(0, 2)
        .join('.'))
    if (sorted.length > 0) {
      Reflect.set(this, 'abi', sorted[0])
    }

    const hash = crypto.createHash('sha256')
    hash.update(JSON.stringify([source, pkgJson.name]))
    const id = `${source}:${pkgJson.name}:${hash.digest('hex').slice(0, 8)}`
    Reflect.set(this, 'id', id)
  }
}

module.exports = Package
