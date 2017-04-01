import Component from './component'
import config from './config'
import glob from 'glob'
import jsonfile from 'jsonfile'
import log from 'electron-log'
import objpath from 'object-path'
import path from 'path'
import semver from 'semver'

export default class Parcel {
  static list () {
    const builtinParcelPattern =
      path.join(config.builtinParcelPath, '/**/package.json')
    const userParcelPattern =
      path.join(config.userParcelPath, '/**/package.json')
    const paths = glob.sync(builtinParcelPattern)
      .concat(glob.sync(userParcelPattern))

    const list = []
    for (const json of paths) {
      try {
        list.push(new Parcel(json))
      } catch (err) {
        log.error(`failed to load ${json}: ${err}`)
      }
    }

    return list
  }

  constructor (rootPath) {
    const pkg = jsonfile.readFileSync(rootPath)
    const engine = objpath.get(pkg, 'engines.deplug', null)
    if (engine === null) {
      throw new Error('deplug version required')
    }
    if (!semver.satisfies(config.deplug.version, engine)) {
      throw new Error('deplug version mismatch')
    }
    const components = objpath.get(pkg, 'deplugParcel.components', [])
    this.components = []
    for (const comp of components) {
      this.components.push(Component.create(rootPath, comp))
    }
  }
}
