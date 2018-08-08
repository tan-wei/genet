import BaseLoader from './base'
import { CompositeDisposable } from '../disposable'
import Style from '../style'
import path from 'path'

export namespace StyleComponent {
  export interface Config {
    files: string[]
  }

  export class Loader implements BaseLoader {
    private styleFiles: string[]
    private disposable: CompositeDisposable

    constructor(comp: Config, dir: string) {
      this.styleFiles = comp.files.map((file) => path.resolve(dir, file))
    }
    async load() {
      const loader = new Style('custom')
      const files = await Promise.all(
        this.styleFiles.map((file) => loader.applyCss(document, file)))
      this.disposable = new CompositeDisposable(files)
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