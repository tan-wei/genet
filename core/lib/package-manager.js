import ComponentFactory from './component-factory'
import { EventEmitter } from 'events'
import env from './env'
import glob from 'glob'
import jsonfile from 'jsonfile'
import objpath from 'object-path'
import path from 'path'
import { promisify } from 'util'
import semver from 'semver'

const promiseGlob = promisify(glob)
const promiseReadFile = promisify(jsonfile.readFile)
const fields = Symbol('fields')
async function readFile (filePath) {
    try {
      const data = await promiseReadFile(filePath)
      return {
        data,
        filePath,
      }
    } catch (err) {
      return {}
    }
}


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
    const danglingPackages = new Set()

    const pkgs = (await Promise.all(
      builtinPaths.map(readFile).concat(userPaths.map(readFile))))
      .filter((pkg) => pkg.data)
    for (const pkg of pkgs) {
      let cache = packages.get(pkg.name)
      if (typeof cache === 'undefined') {
        addedPackages.add(pkg.name)
        cache = {
          disabled: disabledPackages.has(pkg.name),
          components: [],
        }
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
      cache.dir = path.dirname(pkg.filePath)
      packages.set(pkg.name, cache)
      removedPackages.delete(pkg.name)
    }

    const task = []
    Array.from(disabledPackages)
      .concat(Array.from(removedPackages))
      .concat(Array.from(updatedPackages))
      .map((name) => packages.get(name))
      .filter((pkg) => typeof pkg !== 'undefined')
      .forEach((pkg) => {
        for (const comp of pkg.components) {
          task.push(comp.unload().then((result) => {
            if (!result) {
              danglingPackages.add(pkg.data.name)
            }
            return result
          }))
        }
      })

    Array.from(addedPackages)
      .concat(Array.from(updatedPackages))
      .map((name) => packages.get(name))
      .forEach((pkg) => {
        const components = objpath.get(pkg.data, 'deplug.components', [])
        pkg.components = components.map(
          (comp) => ComponentFactory.create(comp, pkg.dir))
      })

    Array.from(enabledPackages)
      .concat(Array.from(addedPackages))
      .concat(Array.from(updatedPackages))
      .map((name) => packages.get(name))
      .forEach((pkg) => {
        for (const comp of pkg.components) {
          task.push(comp.load().then((result) => {
            if (!result) {
              danglingPackages.add(pkg.data.name)
            }
            return result
          }))
        }
      })

    for (const name of danglingPackages) {
      const pkg = packages.get(name)
      if (typeof pkg !== 'undefined') {
        pkg.dangling = true
      }
    }

    for (const name of removedPackages) {
      packages.delete(name)
    }

    await Promise.all(task)
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
