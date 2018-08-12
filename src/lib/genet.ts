import Action from './action'
import Config from './config'
import KeyBind from './keybind'
import Logger from './logger'
import Menu from './menu'
import Notification from './notification'
import PackageManager from './package-manager'
import Resumer from './resumer'
import Session from './session'
import Workspace from './workspace'
import Env from './env'
import minimist from 'minimist'
import Gpm from './gpm'

export default class Genet {
  readonly config: any
  readonly gpm: Gpm
  readonly workspace: Workspace
  readonly keybind: KeyBind
  readonly packages: PackageManager
  readonly resumer?: Resumer
  readonly session: Session
  readonly menu: Menu
  readonly notify: Notification
  readonly logger: Logger
  readonly env: Env
  readonly action: Action
  readonly argv: object

  constructor(argv) {
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
    this.config = config
    this.gpm = new Gpm()
    this.workspace = new Workspace(options.profile)
    this.keybind = new KeyBind(options.profile, logger)
    this.packages = new PackageManager(config, components.split(','), logger)
    this.session = new Session(config)
    this.menu = new Menu()
    this.notify = new Notification()
    this.logger = logger
    this.env = Env
    this.action = new Action()
    this.argv = Object.assign(argv, options)

    if (options.resume) {
      this.resumer = new Resumer(options.resume, logger)
    }

    if (options.contextMenu) {
      this.menu.enableContextMenu()
    }
  }

  init() {}
}
