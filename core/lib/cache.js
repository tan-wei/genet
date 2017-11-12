import crypto from 'crypto'
import env from './env'
import fs from 'fs'
import glob from 'glob'
import path from 'path'
import promisify from 'es6-promisify'
import writeFileAtomic from 'write-file-atomic'

const promiseGlob = promisify(glob)
const promiseReadFile = promisify(fs.readFile)
const promiseUnlink = promisify(fs.unlink)
const promiseWriteFile = promisify(writeFileAtomic)
const fields = Symbol('fields')
async function readExpiryFile (filePath) {
    try {
      return {
        filePath,
        expiry: Number.parseInt(
          await promiseReadFile(`${filePath}.expiry`), 10),
      }
    } catch (err) {
      return {
 filePath,
expiry: 0,
}
    }
}

export default class Cache {
  constructor (profile) {
    const table = new Map()
    const separator = Buffer.from([0])
    this[fields] = {
      getFileName: (key) => {
        let name = table.get(key)
        if (typeof name !== 'undefined') {
          return name
        }
        const hash = crypto.createHash('md5')
        hash.update(profile)
        hash.update(separator)
        hash.update(key)
        name = path.join(env.cachePath, `${hash.digest('hex')}.cache`)
        table.set(key, name)
        return name
      },
    }
  }

  async get (key, defaultValue) {
    const { getFileName } = this[fields]
    try {
      return await promiseReadFile(getFileName(key))
    } catch (err) {
      return defaultValue
    }
  }

  set (key, value, ttl = 3600) {
    const { getFileName } = this[fields]
    const expiry = Date.now() + ttl
    return Promise.all([
      promiseWriteFile(getFileName(key), value),
      promiseWriteFile(`${getFileName(key)}.expiry`, `${expiry}`)
    ])
  }

  static async cleanup () {
    const caches = await promiseGlob(path.join(env.cachePath, '*.cache'))
    const expiries =
      await Promise.all(caches.map((file) => readExpiryFile(`${file}`)))
    const now = Date.now()
    return Promise.all(expiries.map(({ filePath, expiry }) => {
      if (expiry > 0 && expiry < now) {
        return Promise.all([
          promiseUnlink(filePath),
          promiseUnlink(`${filePath}.expiry`)
        ])
      }
      return Promise.resolve()
    }))
  }
}
