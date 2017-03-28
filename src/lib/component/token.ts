import BaseComponent from './base'
import { CompositeDisposable } from '../disposable'
import genet from '@genet/api'
import objpath from 'object-path'
import path from 'path'
import { readJson } from 'fs-extra'

export default class TokenComponent implements BaseComponent {
  private disposable: CompositeDisposable
  private tokenFiles: string[]

  constructor(comp: any, dir: string) {
    this.tokenFiles =
      objpath.get(comp, 'files', []).map((file) => path.resolve(dir, file))
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
