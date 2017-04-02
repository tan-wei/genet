import Parcel from './parcel'
import Theme from './theme'
import config from './config'
import module from 'module'

export default async function (argv) {
  const deplug = {
    Argv: argv,
    Config: config,
    Parcel,
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

  await Parcel.loadComponents('theme')
  return deplug
}
