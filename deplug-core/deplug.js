/* eslint no-underscore-dangle: ["error", { "allow": ["_load"] }] */

import Parcel from './parcel'
import Theme from './theme'
import config from './config'
import module from 'module'

export default async function (argv) {
  const deplug = {
    Argv: argv,
    Config: config,
    Parcel,
    Theme,
  }

  const load = module._load
  module._load = (request, parent, isMain) => {
    if (request === 'deplug') {
      return deplug
    }
    return load(request, parent, isMain)
  }

  const parcels = Parcel.list()
  const tasks = []
  for (const parcel of parcels) {
    for (const comp of parcel.components) {
      tasks.push(comp.load())
    }
  }
  await Promise.all(tasks)
}
