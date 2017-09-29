import Config from './config'
import GlobalChannel from './global-channel'
import Profile from './profile'
import fs from 'fs'
import less from 'less'
import path from 'path'
import { promisify } from 'util'

const themeCachePath = path.join(Config.userPath, '.theme-cache.json')

const globalRegistry = {}
let globalCache = {}
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

    promisify(fs.readFile)(themeCachePath,
        { encoding: 'utf8' }).then((json) => {
      for (const file of JSON.parse(json)) {
        this.render(file)
      }
    })
    .catch()
  }

  async render (lessFile) {
    if (lessFile in globalCache) {
      return globalCache[lessFile]
    }
    const options = {
      paths: [
        path.dirname(lessFile),
        path.join(__dirname, 'theme'),
        path.join(__dirname, '../font-awesome/less'),
        Profile.current.dir
      ],
      plugins: [new LessPlugin()],
      filename: lessFile,
      compress: true,
      globalVars: { 'node-platform': process.platform },
    }
    const code = await promisify(fs.readFile)(lessFile, { encoding: 'utf8' })
    const result = less.render(code, options)
    globalCache[lessFile] = result
    promisify(fs.writeFile)(themeCachePath,
      JSON.stringify(Object.keys(globalCache)), { encoding: 'utf8' })
    return result
  }

  static get current () {
    const id = Theme.currentId
    if (id in globalRegistry) {
      return globalRegistry[id]
    }
    return globalRegistry.default
  }

  static get currentId () {
    return Profile.current.get('_', 'theme', 'default')
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
  if (id in globalRegistry && Profile.current.get('_', 'theme') !== id) {
    globalCache = {}
    Profile.current.set('_', 'theme', id)
    GlobalChannel.emit('core:theme:updated', id)
  }
})
