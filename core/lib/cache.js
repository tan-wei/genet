import crypto from 'crypto'
import env from './env'
import fs from 'fs'
import path from 'path'
import { promisify } from 'util'
import writeFileAtomic from 'write-file-atomic'

const promiseReadFile = promisify(fs.readFile)
const promiseWriteFile = promisify(writeFileAtomic)
const fields = Symbol('fields')
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

  set (key, value) {
    const { getFileName } = this[fields]
    return promiseWriteFile(getFileName(key), value)
  }
}
