import BaseComponent from './base'
import exists from 'file-exists'
import objpath from 'object-path'
import path from 'path'

export default class FileComponent extends BaseComponent {
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
      case 'core:file:importer':
        this.type = 'importer'
        break
      case 'core:file:exporter':
        this.type = 'exporter'
        break
      default:
        throw new Error(`unknown type: ${comp.type}`)
    }
  }
  async load () {
    if (this.type === 'importer') {
      this.disposable =
        deplug.session.registerImporter(global.require(this.mainFile).importer)
    } else if (this.type === 'exporter') {
      this.disposable =
        deplug.session.registerExporter(global.require(this.mainFile).exporter)
    }
    return true
  }
  async unload () {
    if (this.disposable) {
      this.disposable.dispose()
      this.disposable = null
    }
    return true
  }
}
