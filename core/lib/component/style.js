import BaseComponent from './base'
import { Disposable } from 'disposables'
import fs from 'fs'
import less from 'less'
import objpath from 'object-path'
import path from 'path'
import promisify from 'es6-promisify'

const promiseReadFile = promisify(fs.readFile)
export default class StyleComponent extends BaseComponent {
  constructor (comp, dir) {
    super()
    this.styleFiles =
      objpath.get(comp, 'files', []).map((file) => path.resolve(dir, file))
  }
  async load () {
    const files = await Promise.all(
      this.styleFiles.map((file) => promiseReadFile(file, 'utf8')))
    const results = await Promise.all(files.map((data) => less.render(data)))
    const styleTag = document.createElement('style')
    styleTag.textContent = results.map((result) => result.css).join('\n')
    document.head.appendChild(styleTag)
    this.disposable = new Disposable(() => {
      styleTag.remove()
    })
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
