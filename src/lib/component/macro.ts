import BaseLoader from './base'
import { Disposable } from '../disposable'
import Script from '../script'
import genet from '@genet/api'
import path from 'path'

export namespace MacroComponent {
  export interface Config {
    main: string
    name?: string
    description?: string
  }

  export class Loader implements BaseLoader {
    private name: string
    private description: string
    private mainFile: string
    private disposable: Disposable

    constructor(comp: Config, dir: string) {
      if (!comp.main) {
        throw new Error('main field required')
      }
      this.mainFile = path.resolve(dir, comp.main)
      this.name = comp.name || ''
      this.description = comp.description || ''
    }
    async load() {
      const module = await Script.execute(this.mainFile)
      this.disposable = genet.session.registerFilterMacro({
        name: this.name,
        description: this.description,
        func: module,
      })
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
