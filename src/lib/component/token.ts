import BaseLoader from './base'
import { CompositeDisposable } from '../disposable'
import genet from '@genet/api'
import objpath from 'object-path'
import path from 'path'
import { readJson } from 'fs-extra'

export namespace TokenComponent {
  export interface Config {
    files: string[]
  }

  export class Loader implements BaseLoader {
    private disposable: CompositeDisposable
    private tokenFiles: string[]

    constructor(comp: Config, dir: string) {
      this.tokenFiles = comp.files.map((file) => path.resolve(dir, file))
    }
    async load() {
      const tokenList =
        await Promise.all(this.tokenFiles.map((file) => readJson(file)))
      this.disposable = new CompositeDisposable(
        tokenList.map((tokens) => genet.session.registerTokens(tokens)))
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
