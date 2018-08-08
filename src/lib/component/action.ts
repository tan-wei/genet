import BaseLoader from './base'
import { CompositeDisposable } from '../disposable'
import genet from '@genet/api'
import path from 'path'
import { readJson } from 'fs-extra'

export namespace ActionComponent {
  export interface Config {
    files: string[]
  }

  export class Loader implements BaseLoader {
    private disposable: CompositeDisposable
    private files: string[]

    constructor(comp: Config, dir: string) {
      this.files = comp.files.map((file) => path.resolve(dir, file))
    }
    async load() {
      const actionList =
        await Promise.all(this.files.map((file) => readJson(file)))
      this.disposable = new CompositeDisposable(
        actionList.map((actions) => genet.session.registerActions(actions)))
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
