import { readJson, remove, ensureDir } from 'fs-extra'
import ComponentFactory from './component-factory'
import Config from './config'
import { EventEmitter } from 'events'
import Logger from './logger'
import Env from './env'
import genet from '@genet/api'
import glob from 'glob'
import objpath from 'object-path'
import path from 'path'
import { promisify } from 'util'
import semver from 'semver'
import writeFileAtomic from 'write-file-atomic'

const promiseGlob = promisify(glob)
const promiseWriteFile = promisify(writeFileAtomic)
async function readFile(filePath) {
  try {
    const normPath = path.normalize(filePath)
    const data = await readJson(normPath)
    const dir = path.dirname(normPath)
    const builtin = !normPath.startsWith(Env.userPackagePath)
    const id = builtin
      ? `builtin/${path.basename(dir)}`
      : path.relative(Env.userPackagePath, dir)
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
  private _config: Config
  private _logger: Logger
  private _packages: Map<string, any>
  private _activatedComponents: Set<any>
  private _updating: boolean
  private _queued: boolean
  private _initialLoad: boolean

  constructor(config: Config, components: string[], logger: Logger) {
    super()
    this._config = config
    this._logger = logger
    this._packages = new Map()
    this._activatedComponents = new Set(components)
    this._updating = false
    this._queued = false
    this._initialLoad = true
    config.watch('_.disabledPackages', () => {
      this.triggerUpdate()
    }, [])
  }

  triggerUpdate() {
    this.update().catch((err) => {
      this._logger.error(err)
    })
  }

  async update() {
    if (this._updating && !this._queued) {
      this.once('updated', () => {
        this._queued = false
        this.triggerUpdate()
      })
      this._queued = true
      return
    }
    this._updating = true

    const builtinPluginPattern =
      path.join(Env.builtinPackagePath, '/**/package.json')
    const userPluginPattern =
      path.join(Env.userPackagePath, '/**/package.json')

    const globOptions = { ignore: '**/node_modules/*/package.json' }
    const builtinPaths = await promiseGlob(builtinPluginPattern, globOptions)
    const userPaths = await promiseGlob(userPluginPattern, globOptions)

    const disabledPackages = new Set(this._config.get('_.disabledPackages', []))
    const updatedPackages = new Set()
    const enabledPackages = new Set()
    const addedPackages = new Set()
    const removedPackages = new Set(this._packages.keys())
    const dirtyPackages = new Set()

    const metaDataList = await Promise.all(
      builtinPaths.map(readFile).concat(userPaths.map(readFile)))
    for (const data of metaDataList) {
      if (data.err) {
        genet.logger.error(data.err)
      }
    }
    const pkgs = metaDataList.filter((pkg) => pkg.data)
    for (const pkg of pkgs) {
      const id = pkg.id || ''
      const incompatible = !semver.satisfies(
        semver.coerce(Env.genet.version),
        objpath.get(pkg.data, 'engines.genet', '*'))
      const disabled = disabledPackages.has(id)
      const cache = this._packages.get(id) || { components: [] }
      if (!incompatible) {
        if (!this._packages.has(id) && !disabled) {
          addedPackages.add(id)
        } else if (disabled) {
          if (cache.disabled === true) {
            disabledPackages.delete(id)
          } else {
            cache.disabled = true
          }
        } else if (cache.disabled === true) {
          enabledPackages.add(pkg.id)
          cache.disabled = false
        } else if (semver.neq(pkg.data.version, cache.data.version)) {
          updatedPackages.add(pkg.id)
        }
      }

      this._packages.set(id, Object.assign(cache, pkg, { incompatible }))
      removedPackages.delete(id)
    }

    const task: object[] = []
    Array.from(disabledPackages)
      .concat(Array.from(removedPackages))
      .concat(Array.from(updatedPackages))
      .map((id: string) => this._packages.get(id))
      .filter((pkg) => typeof pkg !== 'undefined')
      .forEach((pkg) => {
        for (const comp of pkg.components) {
          this._logger.debug(`unloading package: ${pkg.id}`)
          task.push(comp.unload().then((result) => {
            if (!result) {
              dirtyPackages.add(pkg.id)
            }
            return result
          })
            .catch((err) => {
              this._logger.error(err)
            }))
        }
      })

    Array.from(addedPackages)
      .concat(Array.from(updatedPackages))
      .map((id) => this._packages.get(id))
      .forEach((pkg) => {
        const components = objpath.get(pkg.data, 'genet.components', [])
        pkg.components = components
          .filter((comp) => this._activatedComponents.has(comp.type))
          .map((comp) => {
            try {
              return ComponentFactory.create(comp, pkg.dir)
            } catch (err) {
              this._logger.error(err)
              return null
            }
          })
          .filter((comp) => comp)
      })

    Array.from(enabledPackages)
      .concat(Array.from(addedPackages))
      .concat(Array.from(updatedPackages))
      .map((id) => this._packages.get(id))
      .forEach((pkg) => {
        for (const comp of pkg.components) {
          this._logger.debug(`loading package: ${pkg.id}`)
          task.push(comp.load().then((result) => {
            if (!result && !this._initialLoad) {
              dirtyPackages.add(pkg.id)
            }
            return result
          })
            .catch((err) => {
              this._logger.error(err)
            }))
        }
      })

    for (const id of dirtyPackages) {
      const pkg = this._packages.get(id)
      if (typeof pkg !== 'undefined') {
        pkg.dirty = true
      }
    }

    for (const id of removedPackages) {
      this._packages.delete(id)
    }

    for (const [, pkg] of this._packages) {
      if (pkg.configSchemaDisposer) {
        pkg.configSchemaDisposer.dispose()
      }
      const configSchema = objpath.get(pkg.data, 'genet.configSchema')
      if (typeof configSchema === 'object') {
        pkg.configSchemaDisposer =
          genet.config.registerSchema(configSchema)
      }
    }

    await Promise.all(task)
    this.emit('updated')
    this._updating = false
    this._initialLoad = false
  }

  get list() {
    return Array.from(this._packages.values())
  }

  get(id: string) {
    return this._packages.get(id)
  }

  enable(id: string) {
    const disabledPackages = new Set(this._config.get('_.disabledPackages', []))
    if (disabledPackages.delete(id)) {
      this._config.set('_.disabledPackages', Array.from(disabledPackages))
      this.triggerUpdate()
    }
  }

  disable(id: string) {
    const disabledPackages = new Set(this._config.get('_.disabledPackages', []))
    if (!disabledPackages.has(id)) {
      disabledPackages.add(id)
      this._config.set('_.disabledPackages', Array.from(disabledPackages))
      this.triggerUpdate()
    }
  }

  async uninstall(id: string) {
    const pkg = this.get(id)
    if (typeof pkg !== 'undefined') {
      await remove(pkg.dir)
      this.triggerUpdate()
    }
  }

  static async init() {
    await ensureDir(Env.userPackagePath)
    await ensureDir(Env.cachePath)

    const versionFile = path.join(Env.userPath, '.version')
    await promiseWriteFile(versionFile, JSON.stringify({
      genet: semver.valid(semver.coerce(Env.genet.version)),
      target: process.env.GENET_TARGET,
      resourcePath: path.resolve(__dirname, '../..'),
    }))
  }

  static async cleanup() {
    const userPluginPattern =
      path.join(Env.userPackagePath, '/**/package.json')
    const userPaths = await promiseGlob(userPluginPattern)
    const files = await Promise.all(userPaths.map((file) => {
      const removeme = path.join(path.dirname(file), '.removeme')
      return readFile(removeme)
    }))
    const dirs = files.map((data) => {
      if (data.data && data.filePath) {
        return path.dirname(data.filePath)
      }
      return ''
    }).filter((dir) => dir !== '')
    return Promise.all(dirs.map((dir) => remove(dir)))
  }
}
