import Config from './config'
import KeyBind from './keybind'
import Menu from './menu'
import minimist from 'minimist'

const fields = Symbol('fields')
export default class Deplug {
  constructor (argv) {
    const options = minimist(argv)
    this[fields] = {
      config: new Config(options.profile, 'config'),
      layout: new Config(options.profile, 'layout'),
      keybind: new KeyBind(options.profile),
      menu: new Menu(),
      argv,
    }
  }

  get config () {
    return this[fields].config
  }

  get layout () {
    return this[fields].layout
  }

  get keybind () {
    return this[fields].keybind
  }

  get menu () {
    return this[fields].menu
  }

  get argv () {
    return this[fields].argv
  }
}
