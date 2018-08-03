import BaseComponent from './base'
import { Disposable } from '../disposable'
import Script from '../script'
import genet from '@genet/api'
import objpath from 'object-path'
import path from 'path'

interface Filter {
  name: string
  extensions: string[]
}

export default class FileComponent implements BaseComponent {
  private mainFile: string
  private filters: Filter[]
  private disposable: Disposable

  constructor(comp: any, dir: string) {
    const file = objpath.get(comp, 'main', '')
    if (!file) {
      throw new Error('main field required')
    }
    this.mainFile = path.resolve(dir, file)
    this.filters = objpath.get(comp, 'filters', [])
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
