import BaseLoader from './base'
import { Disposable } from '../disposable'
import Script from '../script'
import genet from '@genet/api'
import path from 'path'

interface Filter {
  name: string
  extensions: string[]
}

export namespace FileComponent {
  export interface Config {
    main: string
    filters?: Filter[]
  }

  export class Loader implements BaseLoader {
    private mainFile: string
    private filters: Filter[]
    private disposable: Disposable

    constructor(comp: Config, dir: string) {
      if (!comp.main) {
        throw new Error('main field required')
      }
      this.mainFile = path.resolve(dir, comp.main)
      this.filters = comp.filters || []
    }
    async load() {
      const handler = await Script.execute(this.mainFile)
      this.disposable = genet.session.registerFileReader({
        handler,
        filters: this.filters,
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
