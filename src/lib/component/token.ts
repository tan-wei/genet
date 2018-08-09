import BaseLoader from './base'
import { Disposable } from '../disposable'
import genet from '@genet/api'
import path from 'path'
import { readJson } from 'fs-extra'

export namespace TokenComponent {
  export interface Config {
    main: string
  }

  export class Loader implements BaseLoader {
    private disposable: Disposable
    private tokenFile: string

    constructor(comp: Config, dir: string) {
      this.tokenFile = path.resolve(dir, comp.main)
    }
    async load() {
      this.disposable = genet.session.registerTokens(readJson(this.tokenFile))
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
