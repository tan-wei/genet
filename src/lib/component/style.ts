import BaseLoader from './base'
import { Disposable } from '../disposable'
import Style from '../style'
import path from 'path'

export namespace StyleComponent {
  export interface Config {
    main: string
  }

  export class Loader implements BaseLoader {
    private styleFile: string
    private disposable: Disposable

    constructor(comp: Config, dir: string) {
      this.styleFile = path.resolve(dir, comp.main)
    }
    async load() {
      const loader = new Style('custom')
      this.disposable = await loader.applyCss(document, this.styleFile)
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