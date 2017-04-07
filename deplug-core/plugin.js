import ComponentFactory from './components/factory'
import config from './config'
import glob from 'glob'
import jsonfile from 'jsonfile'
import objpath from 'object-path'
import path from 'path'
import semver from 'semver'

let plugins = null
export default class Plugin {
  static async loadComponents (type) {
    if (plugins === null) {
      plugins = this.listPlugins()
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

  static listPlugins () {
    const builtinPluginPattern =
      path.join(config.builtinPluginPath, '/**/package.json')
    const userPluginPattern =
      path.join(config.userPluginPath, '/**/package.json')
    const paths = glob.sync(builtinPluginPattern)
      .concat(glob.sync(userPluginPattern))

    const list = []
    for (const root of paths) {
      list.push(new Plugin(path.dirname(root)))
    }
    return list
  }

  constructor (rootDir) {
    const parc = jsonfile.readFileSync(path.join(rootDir, 'package.json'))
    const engine = objpath.get(parc, 'engines.deplug', null)
    if (engine === null) {
      throw new Error('deplug version required')
    }
    if (!semver.satisfies(config.deplug.version, engine)) {
      throw new Error('deplug version mismatch')
    }
    const components = objpath.get(parc, 'deplugin.components', [])
    this.components = []
    for (const comp of components) {
      this.components.push(ComponentFactory.create(rootDir, parc, comp))
    }
  }
}
