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

  await Parcel.loadComponents('theme')
}
