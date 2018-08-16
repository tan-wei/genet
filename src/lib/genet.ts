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
  readonly resumer: Resumer
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
    this.resumer = new Resumer(argv.resume || Resumer.generateFileName(), logger)
    this.menu = new Menu()
    this.notify = new Notification()
    this.logger = logger
    this.env = Env
    this.action = new Action()
    this.argv = argv

    this.keybind.register({
      '*': {
        'command+1': 'core:tab:active:1',
        'command+2': 'core:tab:active:2',
        'command+3': 'core:tab:active:3',
        'command+4': 'core:tab:active:4',
        'command+5': 'core:tab:active:5',
        'command+6': 'core:tab:active:6',
        'command+7': 'core:tab:active:7',
        'command+8': 'core:tab:active:8',
        'command+9': 'core:tab:active:9',
      }
    })

    if (argv.contextMenu) {
      this.menu.enableContextMenu()
    }
  }

  init(genet: Genet) { }
}
