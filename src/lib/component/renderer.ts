import BaseLoader from './base'
import { CompositeDisposable } from '../disposable'
import Script from '../script'
import genet from '@genet/api'
import path from 'path'

export namespace RendererComponent {
  export interface Config {
    main: string
    id: string
    type: string
  }

  export class Loader implements BaseLoader {
    private id: string
    private mainFile: string
    private type: string
    private disposable: CompositeDisposable

    constructor(comp: Config, dir: string) {
      if (!comp.main) {
        throw new Error('main field required')
      }
      if (!comp.id) {
        throw new Error('id field required')
      }
      this.id = comp.id
      this.mainFile = path.resolve(dir, comp.main)
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
    }
    async load() {
      const component = await Script.execute(this.mainFile)
      if (this.type === 'attr') {
        this.disposable =
          genet.session.registerAttrRenderer(this.id, component)
      } else if (this.type === 'layer') {
        this.disposable =
          genet.session.registerLayerRenderer(this.id, component)
      }
      return true
    }
    async unload() {
      if (this.disposable) {
        this.disposable.dispose()
      }
      return true
    }
  }
}
