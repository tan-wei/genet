import GlobalChannel from './global-channel'
import Profile from './profile'
import denodeify from 'denodeify'
import fs from 'fs'
import less from 'less'
import mkpath from 'mkpath'
import os from 'os'
import path from 'path'
import s2p from 'stream-to-promise'

const globalRegistry = {}
const tmpDir = path.join(os.tmpdir(), `${process.pid}`, 'deplug', 'theme')
let currentThemeId = 'default'
export default class Theme {
  constructor (id, name, lessFile) {
    this.id = id
    this.name = name
    this.lessFile = lessFile
  }

  async render (lessFile) {
    await denodeify(mkpath)(tmpDir)

    const files = { [path.join(tmpDir, 'deplug-theme.less')]: this.lessFile, }
    for (const id in globalRegistry) {
      const theme = globalRegistry[id]
      files[path.join(tmpDir, `deplug-${theme.id}-theme.less`)] = theme.lessFile
    }

    const tasks = []
    for (const dst in files) {
      const read = fs.createReadStream(files[dst])
      const write = fs.createWriteStream(dst)
      tasks.push(s2p(read.pipe(write)))
    }
    await Promise.all(tasks)

    const options = {
      paths: [
        tmpDir,
        path.dirname(lessFile),
        path.join(__dirname, 'theme'),
        path.join(__dirname, '../font-awesome/css'),
        Profile.current.$$dir
      ],
      filename: lessFile,
      compress: true,
      globalVars: { 'node-platform': process.platform, },
    }
    const code = await denodeify(fs.readFile)(lessFile, { encoding: 'utf8', })
    return less.render(code, options)
  }

  static get current () {
    return globalRegistry[currentThemeId]
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
  if (id in globalRegistry && currentThemeId !== id) {
    currentThemeId = id
    GlobalChannel.emit('core:theme:updated', currentThemeId)
  }
})
