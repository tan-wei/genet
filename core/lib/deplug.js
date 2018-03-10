import Action from './action'
import Cache from './cache'
import Config from './config'
import KeyBind from './keybind'
import Logger from './logger'
import Menu from './menu'
import Notification from './notification'
import PackageManager from './package-manager'
import PackageRegistry from './package-registry'
import Resumer from './resumer'
import Session from './session'
import Workspace from './workspace'
import minimist from 'minimist'

export default class Deplug {
  constructor (argv) {
    const options = minimist(argv, { boolean: true })
    let components = options.components || ''
    if (Array.isArray(components)) {
      components = components.join(',')
    }
    const config = new Config(options.profile, 'config')
    const logger = new Logger(config)
    if (options.loggerDomain) {
      logger.domain = options.loggerDomain
    }
    const cache = new Cache(options.profile, logger)
    this.config = config
    this.workspace = new Workspace(options.profile)
    this.keybind = new KeyBind(options.profile, logger)
    this.packages = new PackageManager(config, components.split(','), logger)
    this.registry = new PackageRegistry(options.profile, config, cache)
    this.session = new Session(config)
    this.menu = new Menu()
    this.notify = new Notification()
    this.logger = logger
    this.cache = cache
    this.action = new Action()
    this.argv = Object.assign(argv, options)
    if (options.resume) {
      this.resumer = new Resumer(options.resume, logger)
    }

    this.registry.on('error', (err) => {
      logger.warn(err)
    })

    if (options.contextMenu) {
      this.menu.enableContextMenu()
    }

    Object.freeze(this)
  }
}
