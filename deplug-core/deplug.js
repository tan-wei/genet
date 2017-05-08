import { EventEmitter } from 'events'
import GlobalChannel from './global-channel'
import File from './file'
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
    File,
    GlobalChannel,
    Menu: new Menu(),
    Panel,
    Plugin,
    Profile,
    Session,
    Tab,
    Theme,
  }

  /* eslint-disable no-underscore-dangle */
  const load = module._load
  module._load = (request, parent, isMain) => {
    if (request === 'deplug') {
      return deplug
    }
    return load(request, parent, isMain)
  }
  /* eslint-enable no-underscore-dangle */

  await Plugin.loadComponents('theme')
  return deplug
}
