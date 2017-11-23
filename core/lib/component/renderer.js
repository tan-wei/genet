import BaseComponent from './base'
import fs from 'fs'
import objpath from 'object-path'
import path from 'path'
import promisify from 'es6-promisify'
import vm from 'vm'

const promiseReadFile = promisify(fs.readFile)
export default class RendererComponent extends BaseComponent {
  constructor (comp, dir) {
    super()
    const file = objpath.get(comp, 'main', '')
    if (!file) {
      throw new Error('main field required')
    }
    this.mainFile = path.resolve(dir, file)
    switch (comp.type) {
      case 'core:renderer:attr':
        this.type = 'attr'
        break
      case 'core:renderer:layer':
        this.type = 'layer'
        break
      default:
        throw new Error(`unknown renderer type: ${comp.type}`)
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
    if (this.type === 'attr') {
      this.disposable = deplug.session.registerAttrRenderer(module.exports)
    } else if (this.type === 'layer') {
      this.disposable = deplug.session.registerLayerRenderer(module.exports)
    }
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
