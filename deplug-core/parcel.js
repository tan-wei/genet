import ComponentFactory from './components/factory'
import config from './config'
import glob from 'glob'
import jsonfile from 'jsonfile'
import log from 'electron-log'
import objpath from 'object-path'
import path from 'path'
import semver from 'semver'

export default class Parcel {
  static async loadComponents (type) {
    const tasks = []
    for (const parcel of parcels) {
      for (const comp of parcel.components) {
        if (comp.type === type) {
          tasks.push(comp.load())
        }
      }
    }
    return Promise.all(tasks)
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
      this.components.push(ComponentFactory.create(rootPath, comp))
    }
  }
}

function listParcels () {
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

const parcels = listParcels()
