import GlobalChannel from './global-channel'
import Profile from './profile'
import denodeify from 'denodeify'
import fs from 'fs'
import less from 'less'
import path from 'path'

const globalRegistry = {}
class FileManager extends less.FileManager {
  supports (filename) {
    return (/^deplug-(\w+-)?theme$/).test(filename)
  }
  loadFile (filename, currentDirectory, options, environment) {
    const id = (/^deplug-(?:(\w+)-)?theme.less$/).exec(filename)[1] ||
      Theme.currentId
    const file = globalRegistry[id].lessFile
    return super.loadFile(file, currentDirectory, options, environment)
  }
}

class LessPlugin {
  install (ls, pluginManager) {
    pluginManager.addFileManager(new FileManager())
  }
}

export default class Theme {
  constructor (id, name, lessFile) {
    this.id = id
    this.name = name
    this.lessFile = lessFile
  }

  async render (lessFile) {
    const options = {
      paths: [
        path.dirname(lessFile),
        path.join(__dirname, 'theme'),
        path.join(__dirname, '../font-awesome/css'),
        Profile.current.$$dir
      ],
      plugins: [new LessPlugin()],
      filename: lessFile,
      compress: true,
      globalVars: { 'node-platform': process.platform, },
    }
    const code = await denodeify(fs.readFile)(lessFile, { encoding: 'utf8', })
    return less.render(code, options)
  }

  static get current () {
    const id = Theme.currentId
    if (id in globalRegistry) {
      return globalRegistry[id]
    }
      return globalRegistry.default

  }

  static get currentId () {
    return Profile.current.theme || 'default'
  }

  static register (theme) {
    globalRegistry[theme.id] = theme
  }

  static unregister (theme) {
    Reflect.deleteProperty(globalRegistry, theme.id)
  }

  static get registry () {
    return globalRegistry
  }
}

GlobalChannel.on('core:theme:set', (event, id) => {
  if (id in globalRegistry && Profile.current.theme !== id) {
    Profile.current.theme = id
    GlobalChannel.emit('core:theme:updated', id)
  }
})
