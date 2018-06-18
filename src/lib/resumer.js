import jsonfile from 'jsonfile'
import { remote } from 'electron'
import tempy from 'tempy'

const fields = Symbol('fields')
export default class Resumer {
  constructor (file, logger) {
    this[fields] = {
      file,
      data: {},
    }
    try {
      this[fields].data = jsonfile.readFileSync(file)
    } catch (err) {
      logger.debug(err.message)
    }
  }

  get (key) {
    return this[fields].data[key]
  }

  set (key, value) {
    this[fields].data[key] = value
  }

  has (key) {
    return (key in this[fields].data)
  }

  reload () {
    const { data, file } = this[fields]
    jsonfile.writeFileSync(file, data)
    remote.getCurrentWebContents().reload()
  }

  static generateFileName () {
    return tempy.file({ extension: 'json' })
  }
}
