const crypto = require('crypto')
const path = require('path')
const fs = require('fs-extra')
class Package {
  constructor (dir, pkgJson, source) {
    Reflect.set(this, 'metadata', pkgJson)
    Reflect.set(this, 'source', source)
    Reflect.set(this, 'dir', dir)

    try {
      const pattern =
        /\[\[package\]\]\sname = "genet-abi"\sversion = "(\d+\.\d+\.\d+)"/
      const cargoLock = fs.readFileSync(path.join(dir, 'Cargo.lock'), 'utf8')
      const result = cargoLock.match(pattern)
      Reflect.set(this, 'abi', result[1])
    } catch (err) {
      Reflect.set(this, 'abi', null)
    }

    const hash = crypto.createHash('sha256')
    hash.update(JSON.stringify([source, pkgJson.name]))
    const id = `${source}:${pkgJson.name}:${hash.digest('hex').slice(0, 8)}`
    Reflect.set(this, 'id', id)
  }
}

module.exports = Package
