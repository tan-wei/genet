import BaseComponent from './base'
import exists from 'file-exists'
import fs from 'fs'
import objpath from 'object-path'
import path from 'path'
import promisify from 'es6-promisify'

const promiseReadFile = promisify(fs.readFile)
export default class DissectorComponent extends BaseComponent {
  constructor (comp, dir) {
    super()
    const file = objpath.get(comp, 'main', '')
    if (!file) {
      throw new Error('main field required')
    }

    const searchPaths = [
      '.',
      'build/Debug',
      'build/Release'
    ]
    for (const spath of searchPaths) {
      const absolute = path.join(dir, spath, file)
      if (exists.sync(absolute)) {
        this.mainFile = absolute
        break
      }
    }
    if (!this.mainFile) {
      throw new Error(`could not resolve ${file} in ${dir}`)
    }

    switch (comp.type) {
      case 'core:dissector:packet':
        this.type = 'packet'
        break
      case 'core:dissector:stream':
        this.type = 'stream'
        break
      default:
        throw new Error(`unknown dissector type: ${comp.type}`)
    }
  }
  async load () {
    const ext = path.extname(this.mainFile)
    switch (ext) {
      case '.node':
        this.disposable = deplug.session.registerNativeDissector({
          type: this.type,
          main: global.require(this.mainFile).dissector,
        })
        break
      case '.js':
        this.disposable = deplug.session.registerDissector({
          type: this.type,
          main: await promiseReadFile(this.mainFile, 'utf8'),
        })
        break
      default:
        throw new Error(`unknown extension type: ${ext}`)
    }
    return false
  }
  async unload () {
    if (this.disposable) {
      this.disposable.dispose()
      this.disposable = null
      return false
    }
    return true
  }
}
