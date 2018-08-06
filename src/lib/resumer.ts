import fs from 'fs-extra'
import tempy from 'tempy'
import Logger from './logger'
const { remote } = require('electron')

const fields = Symbol('fields')
export default class Resumer {
  constructor(file: string, logger: Logger) {
    this[fields] = {
      file,
      data: {},
    }
    try {
      this[fields].data = fs.readJsonSync(file)
    } catch (err) {
      logger.debug(err.message)
    }
  }

  get(key: string) {
    return this[fields].data[key]
  }

  set(key: string, value: any) {
    this[fields].data[key] = value
  }

  has(key: string) {
    return (key in this[fields].data)
  }

  reload() {
    const { data, file } = this[fields]
    fs.writeJsonSync(file, data)
    remote.getCurrentWebContents().reload()
  }

  static generateFileName(): string {
    return tempy.file({ extension: 'json' })
  }
}
