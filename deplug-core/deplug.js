/* eslint no-underscore-dangle: ["error", { "allow": ["_load"] }] */

const module = require('module')
export default function (argv) {
  let deplug = {}
  Object.defineProperties(deplug, { Argv: { value: argv, }, })

  const load = module._load
  module._load = (request, parent, isMain) => {
    if (request === 'deplug') {
      return deplug
    }
    return load(request, parent, isMain)
  }
}
