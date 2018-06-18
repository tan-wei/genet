import fs from 'fs'
import path from 'path'
import { promisify } from 'util'
import vm from 'vm'

const promiseReadFile = promisify(fs.readFile)
export default class Script {
  static async execute (file) {
    const code = await promiseReadFile(file, 'utf8')
    const wrapper =
      `(function(module, require, __filename, __dirname){ ${code} })`
    const options = {
      filename: file,
      displayErrors: true,
    }
    const func = vm.runInThisContext(wrapper, options)
    function req (name) {
      if (name === 'genet') {
        return genet
      }
      return global.require(name)
    }
    const module = {}
    func(module, req, file, path.dirname(file))
    if (typeof module.exports !== 'function') {
      throw new TypeError('module.exports must be a function')
    }
    return module.exports
  }
}
