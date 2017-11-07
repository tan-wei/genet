import env from './env'
import glob from 'glob'
import path from 'path'
import { promisify } from 'util'

const promiseGlob = promisify(glob)
const fields = Symbol('fields')
export default class PackageManager {
  constructor (config) {
    this[fields] = {
      list: [],
      enabledComponents: new Set(),
    }
  }

  async update () {
    const builtinPluginPattern =
      path.join(env.builtinPackagePath, '/**/package.json')
    const userPluginPattern =
      path.join(env.userPackagePath, '/**/package.json')

    const builtinPaths = await promiseGlob(builtinPluginPattern)
    const userPaths = await promiseGlob(userPluginPattern)
    console.log(builtinPaths, userPaths)
  }

  enableComponent (type) {
    this[fields].enabledComponents.add(type)
  }

  get list () {
    return this[fields].list
  }
}
