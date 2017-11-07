import { EventEmitter } from 'events'
import env from './env'
import glob from 'glob'
import jsonfile from 'jsonfile'
import path from 'path'
import { promisify } from 'util'

async function readFile (filePath) {
    try {
      const data = await jsonfile.readFileSync(filePath)
      return {
        data,
        filePath,
      }
    } catch (err) {
      return {}
    }
}

const promiseGlob = promisify(glob)
const fields = Symbol('fields')
export default class PackageManager extends EventEmitter {
  constructor (config) {
    super()
    this[fields] = {
      config,
      packages: new Map(),
      enabledComponents: new Set(),
      updating: false,
    }
    this.update()
    config.watch('_.disabledPackages', () => {
      this.update()
    })
  }

  async update () {
    const { config, updating, packages } = this[fields]
    if (updating) {
      return
    }
    this[fields].updating = true

    const builtinPluginPattern =
      path.join(env.builtinPackagePath, '/**/package.json')
    const userPluginPattern =
      path.join(env.userPackagePath, '/**/package.json')

    const builtinPaths = await promiseGlob(builtinPluginPattern)
    const userPaths = await promiseGlob(userPluginPattern)

    const disabled = new Set()
    for (const name of config.get('_.disabledPackages', [])) {
      disabled.add(name)
    }

    const pkgs = (await Promise.all(
      builtinPaths.map(readFile).concat(userPaths.map(readFile))))
      .filter((pkg) => pkg.data)
    for (const pkg of pkgs) {
      packages.set(pkg.name, pkg.data)
    }

    this.emit('updated')
    this[fields].updating = false
  }

  enableComponent (type) {
    this[fields].enabledComponents.add(type)
  }

  get list () {
    const list = []
    for (const pkg of this[fields].packages.values()) {
      list.push(pkg)
    }
    return list
  }
}
