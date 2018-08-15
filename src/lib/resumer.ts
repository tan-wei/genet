import fs from 'fs-extra'
import tempy from 'tempy'
import Logger from './logger'
const { remote } = require('electron')

export default class Resumer {
  private _file: string
  private _data: any
  constructor(file: string, logger: Logger) {
    this._file = file
    this._data = {}
    try {
      this._data = fs.readJsonSync(file)
    } catch (err) {
      logger.debug(err.message)
    }
  }

  get(key: string) {
    return this._data[key]
  }

  set(key: string, value: any) {
    this._data[key] = value
  }

  has(key: string) {
    return (key in this._data)
  }

  reload() {
    fs.writeJsonSync(this._file, this._data)
    remote.getCurrentWebContents().reload()
  }

  static generateFileName(): string {
    return tempy.file({ extension: 'json' })
  }
}
