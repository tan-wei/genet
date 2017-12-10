import BaseComponent from './base'
import Script from '../script'
import objpath from 'object-path'
import path from 'path'

export default class TransformComponent extends BaseComponent {
  constructor (comp, dir) {
    super()
    const file = objpath.get(comp, 'main', '')
    if (!file) {
      throw new Error('main field required')
    }
    this.mainFile = path.resolve(dir, file)
    switch (comp.type) {
      case 'core:filter:string':
        this.type = 'string'
        break
      case 'core:filter:token':
        this.type = 'token'
        break
      case 'core:filter:ast':
        this.type = 'ast'
        break
      case 'core:filter:template':
        this.type = 'template'
        break
      default:
        throw new Error(`unknown renderer type: ${comp.type}`)
    }
  }
  async load () {
    const module = await Script.execute(this.mainFile)
    this.disposable = deplug.session.registerFilterTransform({
      type: this.type,
      func: module,
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
