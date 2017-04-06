import denodeify from 'denodeify'
import fs from 'fs'
import less from 'less'
import mkpath from 'mkpath'
import os from 'os'
import path from 'path'
import s2p from 'stream-to-promise'

const globalRegistry = {}
const currentThemeId = 'default'
export default class Theme {
  constructor (id, name, lessFile) {
    this.id = id
    this.name = name
    this.lessFile = lessFile
  }

  async render (lessFile) {
    const tmpDir = path.join(os.tmpdir(), `${process.pid}`, 'deplug', 'theme')
    await denodeify(mkpath)(tmpDir)

    const files = { [path.join(tmpDir, 'deplug.theme')]: this.lessFile, }
    for (const id in globalRegistry) {
      const theme = globalRegistry[id]
      files[path.join(tmpDir, `deplug.${theme.id}.theme`)] = theme.lessFile
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
        path.join(__dirname, '../font-awesome/css')
      ],
      filename: lessFile,
      compress: true,
      globalVars: {
        'node-platform': process.platform
      },
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

  static get registry () {
    return globalRegistry
  }
}
