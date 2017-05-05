import { EventEmitter } from 'events'
import GlobalChannel from './global-channel'
import Menu from './menu'
import Panel from './panel'
import Plugin from './plugin'
import Profile from './profile'
import Session from './session'
import Tab from './tab'
import Theme from './theme'
import config from './config'
import module from 'module'

export default async function (argv) {
  const deplug = {
    Argv: argv,
    Channel: new EventEmitter(),
    Config: config,
    GlobalChannel,
    Menu: new Menu(),
    Panel,
    Plugin,
    Profile,
    Session,
    Tab,
    Theme,
  }

  const load = module._load
  module._load = (request, parent, isMain) => {
    if (request === 'deplug') {
      return deplug
    }
    return load(request, parent, isMain)
  }

  return deplug
}
