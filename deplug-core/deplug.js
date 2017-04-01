/* eslint no-underscore-dangle: ["error", { "allow": ["_load"] }] */

import config from './config'
import module from 'module'

export default function (argv) {
  let deplug = {}
  Object.defineProperties(deplug, {
    Argv: { value: argv, },
    Config: { value: config, },
  })

  const load = module._load
  module._load = (request, parent, isMain) => {
    if (request === 'deplug') {
      return deplug
    }
    return load(request, parent, isMain)
  }
}
