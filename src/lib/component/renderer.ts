import BaseComponent from './base'
import { CompositeDisposable } from '../disposable'
import Script from '../script'
import genet from '@genet/api'
import objpath from 'object-path'
import path from 'path'

export default class RendererComponent extends BaseComponent {
  private id: string
  private mainFile: string
  private macroFile: string
  private type: string
  private disposable: CompositeDisposable

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
      case 'core:renderer:attr':
        this.type = 'attr'
        break
      case 'core:renderer:layer':
        this.type = 'layer'
        break
      default:
        throw new Error(`unknown renderer type: ${comp.type}`)
    }
    const macro = objpath.get(comp, 'macro', '')
    if (macro !== '') {
      this.macroFile = path.resolve(dir, macro)
    }
  }
  async load () {
    const component = await Script.execute(this.mainFile)
    if (this.type === 'attr') {
      this.disposable =
        genet.session.registerAttrRenderer(this.id, component)
      if (this.macroFile) {
        const func = await Script.execute(this.macroFile)
        this.disposable = new CompositeDisposable([
          this.disposable,
          genet.session.registerAttrMacro(this.id, func)
        ])
      }
    } else if (this.type === 'layer') {
      this.disposable =
        genet.session.registerLayerRenderer(this.id, component)
    }
    return true
  }
  async unload () {
    if (this.disposable) {
      this.disposable.dispose()
    }
    return true
  }
}
