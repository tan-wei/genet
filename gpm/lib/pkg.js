const crypto = require('crypto')
class Package {
  constructor (dir, pkgJson, source) {
    Reflect.set(this, 'metadata', pkgJson)
    Reflect.set(this, 'source', source)
    Reflect.set(this, 'dir', dir)

    const hash = crypto.createHash('sha256')
    hash.update(JSON.stringify([source, pkgJson.name]))
    const id = `${source}:${pkgJson.name}:${hash.digest('hex').slice(0, 8)}`
    Reflect.set(this, 'id', id)
  }
}

module.exports = Package
