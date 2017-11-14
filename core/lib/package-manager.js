import ComponentFactory from './component-factory'
import { EventEmitter } from 'events'
import env from './env'
import glob from 'glob'
import jsonfile from 'jsonfile'
import objpath from 'object-path'
import path from 'path'
import promisify from 'es6-promisify'
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
  constructor (config, components) {
    super()
    this[fields] = {
      config,
      packages: new Map(),
      activatedComponents: new Set(components),
      updating: false,
      queued: false,
      initialLoad: true,
    }
    this.update()
    config.watch('_.disabledPackages', () => {
      this.update()
    })
  }

  async update () {
    const {
      config, updating, packages,
      activatedComponents, initialLoad, queued,
    } = this[fields]
    if (updating && !queued) {
      this.once('updated', () => {
        this[fields].queued = false
        this.update()
      })
      this[fields].queued = true
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
    const dirtyPackages = new Set()

    const pkgs = (await Promise.all(
      builtinPaths.map(readFile).concat(userPaths.map(readFile))))
      .filter((pkg) => pkg.data)
    for (const pkg of pkgs) {
      const disabled = disabledPackages.has(pkg.data.name)
      const cache = packages.get(pkg.data.name) || { components: [] }
      if (!packages.has(pkg.data.name) && !disabled) {
        addedPackages.add(pkg.data.name)
      } else if (disabled) {
        if (cache.disabled === true) {
          disabledPackages.delete(pkg.data.name)
        } else {
          cache.disabled = true
        }
      } else if (cache.disabled === true) {
        enabledPackages.add(pkg.data.name)
        cache.disabled = false
      } else if (semver.neq(pkg.data.version, cache.data.version)) {
        updatedPackages.add(pkg.data.name)
      }

      cache.data = pkg.data
      cache.dir = path.dirname(pkg.filePath)
      packages.set(pkg.data.name, cache)
      removedPackages.delete(pkg.data.name)
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
              dirtyPackages.add(pkg.data.name)
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
        pkg.components = components
          .filter((comp) => activatedComponents.has(comp.type))
          .map((comp) => ComponentFactory.create(comp, pkg.dir))
      })

    Array.from(enabledPackages)
      .concat(Array.from(addedPackages))
      .concat(Array.from(updatedPackages))
      .map((name) => packages.get(name))
      .forEach((pkg) => {
        for (const comp of pkg.components) {
          task.push(comp.load().then((result) => {
            if (!result && !initialLoad) {
              dirtyPackages.add(pkg.data.name)
            }
            return result
          }))
        }
      })

    for (const name of dirtyPackages) {
      const pkg = packages.get(name)
      if (typeof pkg !== 'undefined') {
        pkg.dirty = true
      }
    }

    for (const name of removedPackages) {
      packages.delete(name)
    }

    for (const [, pkg] of packages) {
      if (pkg.configSchemaDisposer) {
        pkg.configSchemaDisposer.dispose()
      }
      const configSchema = objpath.get(pkg.data, 'deplug.configSchema')
      if (typeof configSchema === 'object') {
        pkg.configSchemaDisposer =
          deplug.config.registerSchema(configSchema)
      }
    }

    await Promise.all(task)
    this.emit('updated')
    this[fields].updating = false
    this[fields].initialLoad = false
  }

  get list () {
    return Array.from(this[fields].packages.values())
  }

  enable (name) {
    const { config } = this[fields]
    const disabledPackages = new Set(config.get('_.disabledPackages', []))
    if (disabledPackages.delete(name)) {
      config.set('_.disabledPackages', Array.from(disabledPackages))
      this.update()
    }
  }

  disable (name) {
    const { config } = this[fields]
    const disabledPackages = new Set(config.get('_.disabledPackages', []))
    if (!disabledPackages.has(name)) {
      disabledPackages.add(name)
      config.set('_.disabledPackages', Array.from(disabledPackages))
      this.update()
    }
  }
}
