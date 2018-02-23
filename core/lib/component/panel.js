import BaseComponent from './base'
import Script from '../script'
import fs from 'fs'
import objpath from 'object-path'
import path from 'path'
import { promisify } from 'util'

const promiseReadFile = promisify(fs.readFile)
export default class PanelComponent extends BaseComponent {
  constructor (comp, dir) {
    super()
    const file = objpath.get(comp, 'main', '')
    if (!file) {
      throw new Error('main field required')
    }
    this.id = objpath.get(comp, 'id', '')
    if (!this.id) {
      throw new Error('id field required')
    }
    this.name = objpath.get(comp, 'name', '')
    if (!this.name) {
      throw new Error('name field required')
    }
    this.mainFile = path.resolve(dir, file)
    const style = objpath.get(comp, 'style', '')
    if (style.length > 0) {
      this.styleFile = path.resolve(dir, style)
    }
  }

  async load () {
    let style = ''
    if (this.styleFile) {
      style = await promiseReadFile(this.styleFile, 'utf8')
    }
    const component = await Script.execute(this.mainFile)
    this.disposable =
      deplug.workspace.registerPanel(this.id, {
        component,
        style,
        id: this.id,
        name: this.name,
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
