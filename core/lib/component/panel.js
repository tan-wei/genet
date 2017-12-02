import BaseComponent from './base'
import Style from '../style'
import fs from 'fs'
import objpath from 'object-path'
import path from 'path'
import promisify from 'es6-promisify'
import vm from 'vm'

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
    this.mainFile = path.resolve(dir, file)
    const style = objpath.get(comp, 'style', '')
    if (style.length > 0) {
      this.styleFile = path.resolve(dir, style)
    }
  }

  async load () {
    const code = await promiseReadFile(this.mainFile, 'utf8')
    const wrapper =
      `(function(module, require, __filename, __dirname){ ${code} })`
    const options = {
      filename: this.mainFile,
      displayErrors: true,
    }
    const func = vm.runInThisContext(wrapper, options)
    function req (name) {
      if (name === 'deplug') {
        return deplug
      }
      return global.require(name)
    }
    const module = {}
    func(module, req, this.mainFile, path.dirname(this.mainFile))
    if (typeof module.exports !== 'function') {
      throw new TypeError('module.exports must be a function')
    }
    let style = ''
    if (this.styleFile) {
      const loader = new Style()
      const result = await loader.compileLess(this.styleFile)
      style = result.css
    }
    this.disposable =
      deplug.workspace.registerPanel(this.id, {
        component: module.exports,
        style,
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
