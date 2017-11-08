import BaseComponent from './base'
import jsonfile from 'jsonfile'
import objpath from 'object-path'
import path from 'path'
import { promisify } from 'util'

const promiseReadFile = promisify(jsonfile.readFile)
async function readFile (filePath) {
    try {
      return await promiseReadFile(filePath)
    } catch (err) {
      return {}
    }
}

export default class TokenComponent extends BaseComponent {
  constructor (comp, dir) {
    super()
    this.tokenFiles =
      objpath.get(comp, 'files', []).map((file) => path.resolve(dir, file))
  }
  async load () {
    const objects = await Promise.all(this.tokenFiles.map(readFile))
    return true
  }
  async unload () {
    return true
  }
}
