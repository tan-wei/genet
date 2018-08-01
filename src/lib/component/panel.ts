import BaseComponent from './base'
import { Disposable } from '../disposable'
import Script from '../script'
import fs from 'fs'
import genet from '@genet/api'
import objpath from 'object-path'
import path from 'path'
import { promisify } from 'util'

const promiseReadFile = promisify(fs.readFile)
export default class PanelComponent implements BaseComponent {
  private id: string
  private name: string
  private slot: string
  private mainFile: string
  private styleFile: string
  private disposable: Disposable

  constructor (comp: any, dir: string) {
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
    this.slot = objpath.get(comp, 'slot', 'tools:bottom')
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
      genet.workspace.registerPanel(this.id, {
        component,
        style,
        slot: this.slot,
        id: this.id,
        name: this.name,
      })
    return true
  }

  async unload () {
    if (this.disposable) {
      this.disposable.dispose()
    }
    return true
  }
}
