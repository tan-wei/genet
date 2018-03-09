import ComponentFactory from './component-factory'
import { EventEmitter } from 'events'
import env from './env'
import fs from 'fs'
import glob from 'glob'
import jsonfile from 'jsonfile'
import mkpath from 'mkpath'
import objpath from 'object-path'
import path from 'path'
import { promisify } from 'util'
import rimraf from 'rimraf'
import semver from 'semver'
import writeFileAtomic from 'write-file-atomic'

const promiseGlob = promisify(glob)
const promiseReadFile = promisify(jsonfile.readFile)
const promiseWriteFile = promisify(writeFileAtomic)
const promiseUnlink = promisify(fs.unlink)
const promiseRmdir = promisify(rimraf)
const promiseMkpath = promisify(mkpath)
const fields = Symbol('fields')
async function readFile (filePath) {
  try {
    const normPath = path.normalize(filePath)
    const data = await promiseReadFile(normPath)
    const dir = path.dirname(normPath)
    const builtin = !normPath.startsWith(env.userPackagePath)
    const id = builtin
      ? `builtin/${path.basename(dir)}`
      : path.relative(env.userPackagePath, dir)
    return {
      data,
      filePath: normPath,
      dir,
      builtin,
      id,
    }
  } catch (err) {
    return { err }
  }
}


export default class PackageManager extends EventEmitter {
  constructor (config, components, logger) {
    super()
    this[fields] = {
      config,
      logger,
      packages: new Map(),
      activatedComponents: new Set(components),
      updating: false,
      queued: false,
      initialLoad: true,
    }
    config.watch('_.disabledPackages', () => {
      this.triggerUpdate()
    }, [])
  }

  triggerUpdate () {
    this.update().catch((err) => {
      this[fields].logger.error(err)
    })
  }

  async update () {
    const {
      config, updating, packages,
      activatedComponents, initialLoad, queued, logger,
    } = this[fields]
    if (updating && !queued) {
      this.once('updated', () => {
        this[fields].queued = false
        this.triggerUpdate()
      })
      this[fields].queued = true
      return
    }
    this[fields].updating = true

    const builtinPluginPattern =
      path.join(env.builtinPackagePath, '/**/package.json')
    const userPluginPattern =
      path.join(env.userPackagePath, '/**/package.json')

    const globOptions = { ignore: '**/node_modules/*/package.json' }
    const builtinPaths = await promiseGlob(builtinPluginPattern, globOptions)
    const userPaths = await promiseGlob(userPluginPattern, globOptions)

    const disabledPackages = new Set(config.get('_.disabledPackages', []))
    const updatedPackages = new Set()
    const enabledPackages = new Set()
    const addedPackages = new Set()
    const removedPackages = new Set(packages.keys())
    const dirtyPackages = new Set()

    const metaDataList = await Promise.all(
      builtinPaths.map(readFile).concat(userPaths.map(readFile)))
    for (const data of metaDataList) {
      if (data.err) {
        deplug.logger.error(data.err)
      }
    }
    const pkgs = metaDataList.filter((pkg) => pkg.data)

    const removeme = await Promise.all(pkgs.map((pkg) =>
      readFile(path.join(path.dirname(pkg.filePath), '.removeme'))))
    for (let index = 0; index < pkgs.length; index += 1) {
      const pkg = pkgs[index]
      if (typeof removeme[index].data === 'undefined') {
        pkg.removal = false
      } else {
        disabledPackages.add(pkg.id)
        pkg.removal = true
      }
    }

    for (const pkg of pkgs) {
      const disabled = disabledPackages.has(pkg.id)
      const cache = packages.get(pkg.id) || { components: [] }
      if (!packages.has(pkg.id) && !disabled) {
        addedPackages.add(pkg.id)
      } else if (disabled) {
        if (cache.disabled === true) {
          disabledPackages.delete(pkg.id)
        } else {
          cache.disabled = true
        }
      } else if (cache.disabled === true) {
        enabledPackages.add(pkg.id)
        cache.disabled = false
      } else if (semver.neq(pkg.data.version, cache.data.version)) {
        updatedPackages.add(pkg.id)
      }

      cache.data = pkg.data
      cache.removal = pkg.removal
      cache.dir = pkg.dir
      cache.id = pkg.id
      cache.builtin = pkg.builtin
      packages.set(pkg.id, cache)
      removedPackages.delete(pkg.id)
    }

    const task = []
    Array.from(disabledPackages)
      .concat(Array.from(removedPackages))
      .concat(Array.from(updatedPackages))
      .map((id) => packages.get(id))
      .filter((pkg) => typeof pkg !== 'undefined')
      .forEach((pkg) => {
        for (const comp of pkg.components) {
          logger.debug(`unloading package: ${pkg.id}`)
          task.push(comp.unload().then((result) => {
            if (!result) {
              dirtyPackages.add(pkg.id)
            }
            return result
          })
            .catch((err) => {
              logger.error(err)
            }))
        }
      })

    Array.from(addedPackages)
      .concat(Array.from(updatedPackages))
      .map((id) => packages.get(id))
      .forEach((pkg) => {
        const components = objpath.get(pkg.data, 'deplug.components', [])
        pkg.components = components
          .filter((comp) => activatedComponents.has(comp.type))
          .map((comp) => ComponentFactory.create(comp, pkg.dir))
      })

    Array.from(enabledPackages)
      .concat(Array.from(addedPackages))
      .concat(Array.from(updatedPackages))
      .map((id) => packages.get(id))
      .forEach((pkg) => {
        for (const comp of pkg.components) {
          logger.debug(`loading package: ${pkg.id}`)
          task.push(comp.load().then((result) => {
            if (!result && !initialLoad) {
              dirtyPackages.add(pkg.id)
            }
            return result
          })
            .catch((err) => {
              logger.error(err)
            }))
        }
      })

    for (const id of dirtyPackages) {
      const pkg = packages.get(id)
      if (typeof pkg !== 'undefined') {
        pkg.dirty = true
      }
    }

    for (const id of removedPackages) {
      packages.delete(id)
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

  get (id) {
    return this[fields].packages.get(id)
  }

  enable (id) {
    const { config } = this[fields]
    const disabledPackages = new Set(config.get('_.disabledPackages', []))
    if (disabledPackages.delete(id)) {
      config.set('_.disabledPackages', Array.from(disabledPackages))
      this.triggerUpdate()
    }
  }

  disable (id) {
    const { config } = this[fields]
    const disabledPackages = new Set(config.get('_.disabledPackages', []))
    if (!disabledPackages.has(id)) {
      disabledPackages.add(id)
      config.set('_.disabledPackages', Array.from(disabledPackages))
      this.triggerUpdate()
    }
  }

  setUninstallFlag (id, flag) {
    const pkg = this.get(id)
    if (typeof pkg !== 'undefined') {
      const removeme = path.join(pkg.dir, '.removeme')
      if (flag) {
        promiseWriteFile(removeme, '{}').then(() => {
          this.triggerUpdate()
        })
      } else {
        promiseUnlink(removeme).then(() => {
          this.triggerUpdate()
        })
      }
    }
  }

  static async init () {
    await promiseMkpath(env.userPackagePath)
    await promiseMkpath(env.cachePath)

    const versionFile = path.join(env.userPath, '.version')
    await promiseWriteFile(versionFile, JSON.stringify({
      deplug: env.deplug.version,
      negatron: env.deplug.devDependencies.negatron,
      abi: process.versions.modules,
      resourcePath: path.resolve(__dirname, '../..'),
    }))
  }

  static async cleanup () {
    const userPluginPattern =
      path.join(env.userPackagePath, '/**/package.json')
    const userPaths = await promiseGlob(userPluginPattern)
    const files = await Promise.all(userPaths.map((file) => {
      const removeme = path.join(path.dirname(file), '.removeme')
      return readFile(removeme)
    }))
    const dirs = files.map((data) => {
      if (data.data) {
        return path.dirname(data.filePath)
      }
      return null
    }).filter((dir) => dir !== null)
    return Promise.all(dirs.map((dir) => promiseRmdir(dir)))
  }
}
