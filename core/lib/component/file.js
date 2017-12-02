import BaseComponent from './base'
import Script from '../script'
import objpath from 'object-path'
import path from 'path'

export default class FileComponent extends BaseComponent {
  constructor (comp, dir) {
    super()
    const file = objpath.get(comp, 'main', '')
    if (!file) {
      throw new Error('main field required')
    }
    this.id = objpath.get(comp, 'id', '')
    if (!this.id) {
      throw new Error('id field required')
    }
    this.mainFile = path.resolve(dir, file)
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
    const component = await Script.execute(this.mainFile)
    if (this.type === 'importer') {
      this.disposable =
        deplug.session.registerImporter(this.id, component)
    } else if (this.type === 'exporter') {
      this.disposable =
        deplug.session.registerExporter(this.id, component)
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
