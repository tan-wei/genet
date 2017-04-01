import config from './config'
import glob from 'glob'
import jsonfile from 'jsonfile'
import path from 'path'

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
      list.push(new Parcel(json))
    }

    return list
  }

  constructor (jsonPath) {
    const pkg = jsonfile.readFileSync(jsonPath)
    return pkg
  }
}
