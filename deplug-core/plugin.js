import config from './config'
import denodeify from 'denodeify'
import glob from 'glob'
import jsonfile from 'jsonfile'
import objpath from 'object-path'
import path from 'path'
import roll from './roll'
import semver from 'semver'

export default class Plugin {
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
    return new Plugin(rootDir, pkg, compList, options)
  }

  constructor (rootDir, pkg, compList, options) {
    this.pkg = pkg
    this.rootDir = rootDir
    this.builtin = !this.pkg.name.startsWith('deplugin-')
    this.compList = compList
    this.options = options
  }
}
