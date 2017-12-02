import BaseComponent from './base'
import Script from '../script'
import objpath from 'object-path'
import path from 'path'

export default class RendererComponent extends BaseComponent {
  constructor (comp, dir) {
    super()
    const file = objpath.get(comp, 'main', '')
    if (!file) {
      throw new Error('main field required')
    }
    this.mainFile = path.resolve(dir, file)
    this.id = objpath.get(comp, 'id', '')
    if (!this.id) {
      throw new Error('id field required')
    }
  }
  async load () {
    const module = await Script.execute(this.mainFile)
    this.disposable = deplug.session.registerFilterTransform({
      id: this.id,
      execute: module,
    })
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
