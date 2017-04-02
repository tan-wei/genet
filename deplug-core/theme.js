import denodeify from 'denodeify'
import fs from 'fs'
import less from 'less'
import mkpath from 'mkpath'
import os from 'os'
import path from 'path'

const globalRegistry = {}
const currentThemeId = 'default'
export default class Theme {
  constructor (id, name, lessFile) {
    this.id = id
    this.name = name
    this.lessFile = lessFile
    this.tmpDir = null
  }

  async render (lessFile) {
    if (this.tmpDir === null) {
      this.tmpDir = path.join(os.tmpdir(), 'deplug', 'theme', this.id)
      const tmpFile = path.join(this.tmpDir, 'deplug.theme')
      await denodeify(mkpath)(this.tmpDir)
      const data = await denodeify(fs.readFile)(this.lessFile)
      await denodeify(fs.writeFile)(tmpFile, data)
    }
    const options = {
      paths: [this.tmpDir, path.dirname(lessFile)],
      filename: lessFile,
      compress: true,
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
