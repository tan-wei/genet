import BaseComponent from './base'
import { CompositeDisposable } from '../disposable'
import Style from '../style'
import objpath from 'object-path'
import path from 'path'

export default class StyleComponent implements BaseComponent {
  private styleFiles: string[]
  private disposable: CompositeDisposable

  constructor (comp: any, dir: string) {
    this.styleFiles =
      objpath.get(comp, 'files', []).map((file) => path.resolve(dir, file))
  }
  async load () {
    const loader = new Style('custom')
    const files = await Promise.all(
      this.styleFiles.map((file) => loader.applyCss(document, file)))
    this.disposable = new CompositeDisposable(files)
    return true
  }
  async unload () {
    if (this.disposable) {
      this.disposable.dispose()
    }
    return true
  }
}
