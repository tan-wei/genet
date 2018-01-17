import BaseComponent from './base'
import { CompositeDisposable } from 'disposables'
import { _load } from 'module'
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

    this.extensions = objpath.get(comp, 'extensions', [])

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
      this.disposable = new CompositeDisposable([
        deplug.session.registerImporter(this.mainFile),
        deplug.session.registerFileImporterExtensions(this.extensions)
      ])
    } else if (this.type === 'exporter') {
      this.disposable = new CompositeDisposable([
        deplug.session.registerExporter(this.mainFile),
        deplug.session.registerFileExporterExtensions(this.extensions)
      ])
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
