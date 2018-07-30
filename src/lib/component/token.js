import BaseComponent from './base'
import { CompositeDisposable } from 'disposables'
import genet from '@genet/api'
import objpath from 'object-path'
import path from 'path'
import { readJson } from 'fs-extra'

export default class TokenComponent extends BaseComponent {
  constructor (comp, dir) {
    super()
    this.tokenFiles =
      objpath.get(comp, 'files', []).map((file) => path.resolve(dir, file))
  }
  async load () {
    const tokenList =
      await Promise.all(this.tokenFiles.map((file) => readJson(file)))
    this.disposable = new CompositeDisposable(
      tokenList.map((tokens) => genet.session.registerTokens(tokens)))
    return true
  }
  async unload () {
    if (this.disposable) {
      this.disposable.dispose()
      this.disposable = null
    }
    return true
  }
}
