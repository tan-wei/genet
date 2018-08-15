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
    let components = argv.components || []
    const config = new Config(argv.profile, 'config')
    const logger = new Logger()
    if (argv.loggerDomain) {
      logger.domain = argv.loggerDomain
    }
    this.config = config
    this.gpm = new Gpm()
    this.workspace = new Workspace(argv.profile)
    this.keybind = new KeyBind(argv.profile, logger)
    this.packages = new PackageManager(config, components, logger)
    this.session = new Session(config)
    this.menu = new Menu()
    this.notify = new Notification()
    this.logger = logger
    this.env = Env
    this.action = new Action()
    this.argv = argv

    if (argv.resume) {
      this.resumer = new Resumer(argv.resume, logger)
    }

    if (argv.contextMenu) {
      this.menu.enableContextMenu()
    }
  }

  init() { }
}
