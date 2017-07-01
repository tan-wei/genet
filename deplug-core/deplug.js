import { EventEmitter } from 'events'
import File from './file'
import GlobalChannel from './global-channel'
import Menu from './menu'
import Panel from './panel'
import Plugin from './plugin'
import PluginLoader from './plugin-loader'
import Profile from './profile'
import Session from './session'
import Tab from './tab'
import Theme from './theme'
import config from './config'
import module from 'module'

export default async function (profile, argv) {
  Profile.currentId = profile
  const deplug = {
    Argv: argv,
    Channel: new EventEmitter(),
    Config: config,
    File,
    GlobalChannel,
    Menu: new Menu(),
    Panel,
    PluginLoader,
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

  await PluginLoader.loadComponents('theme')
  return deplug
}
