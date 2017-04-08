import Channel from './channel'
import Menu from './menu'
import Plugin from './plugin'
import Profile from './profile'
import Tab from './tab'
import Theme from './theme'
import config from './config'
import module from 'module'

export default async function (argv) {
  const deplug = {
    Argv: argv,
    Config: config,
    Channel,
    Menu: new Menu(),
    Plugin,
    Profile,
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
