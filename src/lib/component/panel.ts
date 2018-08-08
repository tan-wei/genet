import BaseLoader from './base'
import { Disposable } from '../disposable'
import Script from '../script'
import fs from 'fs'
import genet from '@genet/api'
import path from 'path'
import { promisify } from 'util'

const promiseReadFile = promisify(fs.readFile)
export namespace PanelComponent {
  export interface Config {
    main: string
    id: string
    name: string
    slot?: string
    style?: string
  }

  export class Loader implements BaseLoader {
    private id: string
    private name: string
    private slot: string
    private mainFile: string
    private styleFile: string
    private disposable: Disposable

    constructor(comp: Config, dir: string) {
      if (!comp.main) {
        throw new Error('main field required')
      }
      if (!comp.id) {
        throw new Error('id field required')
      }
      this.id = comp.id
      if (!comp.name) {
        throw new Error('name field required')
      }
      this.name = comp.name
      this.slot = comp.slot || 'tools:bottom'
      this.mainFile = path.resolve(dir, comp.main)
      const style = comp.style || ''
      if (style) {
        this.styleFile = path.resolve(dir, style)
      }
    }

    async load() {
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

    async unload() {
      if (this.disposable) {
        this.disposable.dispose()
      }
      return true
    }
  }
}
