import BaseComponent from './base'
import jsonfile from 'jsonfile'
import objpath from 'object-path'
import path from 'path'
import promisify from 'es6-promisify'

const promiseReadFile = promisify(jsonfile.readFile)
async function readFile (filePath) {
    try {
      return await promiseReadFile(filePath)
    } catch (err) {
      return {}
    }
}

export default class DissectorComponent extends BaseComponent {
  constructor (comp, dir) {
    super()
  }
  async load () {
    return true
  }
  async unload () {
    return true
  }
}
