import ComponentFactory from './components/factory'
import Profile from './profile'
import config from './config'
import denodeify from 'denodeify'
import glob from 'glob'
import jsonfile from 'jsonfile'
import objpath from 'object-path'
import path from 'path'
import roll from './roll'
import semver from 'semver'

let plugins = null
export default class Plugin {
  static async loadComponents (type) {
    if (plugins === null) {
      plugins = await this.listPlugins()
    }
    const tasks = []
    for (const plugin of plugins) {
      for (const comp of plugin.components) {
        if (comp.type === type) {
          tasks.push(comp.load())
        }
      }
    }
    return Promise.all(tasks)
  }

  static async listPlugins () {
    const builtinPluginPattern =
      path.join(config.builtinPluginPath, '/**/package.json')
    const userPluginPattern =
      path.join(config.userPluginPath, '/**/package.json')

    const builtinPaths = await denodeify(glob)(builtinPluginPattern)
    const userPaths = await denodeify(glob)(userPluginPattern)

    const list = []
    for (const root of builtinPaths.concat(userPaths)) {
      try {
        list.push(Plugin.create(path.dirname(root)))
      } catch (error) {
        // eslint-disable-next-line no-console
        console.error(error)
      }
    }
    return Promise.all(list)
  }

  static async create (rootDir) {
    const pkg = await denodeify(jsonfile.readFile)(
      path.join(rootDir, 'package.json'))
    const engine = objpath.get(pkg, 'engines.deplug', null)
    if (engine === null) {
      throw new Error('deplug version required')
    }
    if (!semver.satisfies(config.deplug.version, engine)) {
      throw new Error('deplug version mismatch')
    }
    const { main } = pkg
    let compList = objpath.get(pkg, 'deplugin.components', [])
    const module = {}
    if (main) {
      const localExtern = Object.keys(objpath.get(pkg, 'dependencies', {}))
      const func = await roll(path.join(rootDir, main), rootDir, localExtern)
      func(module)
      compList = objpath.get(module.exports, 'components', [])
    }
    const options = objpath.get(module.exports, 'options', [])
    for (const opt of options) {
      if ('default' in opt) {
        Profile.setPluginDefault(pkg.name, opt.id, opt.default)
      }
    }
    const components =
      compList.map((comp) => ComponentFactory.create(rootDir, pkg, comp))
    return new Plugin(rootDir, pkg, components, options)
  }

  constructor (rootDir, pkg, comp, options) {
    this.pkg = pkg
    this.components = comp
    this.builtin = this.pkg.name.startsWith('deplug-builtin-')
    this.options = options
  }
}
