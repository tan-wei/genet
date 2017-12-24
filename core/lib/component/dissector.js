import BaseComponent from './base'
import { _load } from 'module'
import exists from 'file-exists'
import objpath from 'object-path'
import path from 'path'

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

    this.linkLayers = objpath.get(comp, 'linkLayers', [])

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
        this.disposable = deplug.session.registerDissector({
          type: this.type,
          main: _load(this.mainFile).dissector,
        })
        break
      case '.js':
        this.disposable = deplug.session.registerDissector({
          type: this.type,
          main: this.mainFile,
        })
        break
      default:
        throw new Error(`unknown extension type: ${ext}`)
    }
    for (const layer of this.linkLayers) {
      deplug.session.registerLinkLayer(layer)
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
