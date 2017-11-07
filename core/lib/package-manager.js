import { EventEmitter } from 'events'
import env from './env'
import glob from 'glob'
import jsonfile from 'jsonfile'
import path from 'path'
import { promisify } from 'util'
import semver from 'semver'

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

    const disabledPackages = new Set(config.get('_.disabledPackages', []))
    const updatedPackages = new Set()
    const enabledPackages = new Set()
    const addedPackages = new Set()
    const removedPackages = new Set(packages.keys())

    const pkgs = (await Promise.all(
      builtinPaths.map(readFile).concat(userPaths.map(readFile))))
      .filter((pkg) => pkg.data)
    for (const pkg of pkgs) {
      let cache = packages.get(pkg.name)
      if (typeof cache === 'undefined') {
        addedPackages.add(pkg.name)
        cache = { disabled: disabledPackages.has(pkg.name) }
      } else if (disabledPackages.has(pkg.name)) {
        if (cache.disabled === true) {
          disabledPackages.delete(pkg.name)
        } else {
          cache.disabled = true
        }
      } else if (cache.disabled === true) {
        enabledPackages.add(pkg.name)
        cache.disabled = false
      } else if (semver.neq(pkg.data.version, cache.data.version)) {
        updatedPackages.add(pkg.name)
      }

      cache.data = pkg.data
      packages.set(pkg.name, cache)
      removedPackages.delete(pkg.name)
    }

    for (const name of disabledPackages) {
      if (!packages.has(name)) {
        disabledPackages.delete(name)
      }
    }

    this.emit('updated')
    this[fields].updating = false
  }

  enableComponent (type) {
    this[fields].enabledComponents.add(type)
  }

  get list () {
    return Array.from(this[fields].packages.values())
  }
}
