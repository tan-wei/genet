import { Disposable } from 'disposables'
import fs from 'fs'
import less from 'less'
import path from 'path'
import promisify from 'es6-promisify'

const fields = Symbol('fields')
const readFile = promisify(fs.readFile)
class FileManager extends less.FileManager {
  constructor (themeFile) {
    super()
    this.themeFile = themeFile
  }
  supports (filename) {
    return filename === 'deplug-theme'
  }
  loadFile (filename, currentDirectory, options, environment) {
    return super
      .loadFile(this.themeFile, currentDirectory, options, environment)
  }
}

class LessPlugin {
  constructor (themeFile) {
    this.themeFile = themeFile
  }
  install (ls, pluginManager) {
    pluginManager.addFileManager(new FileManager(this.themeFile))
  }
}

export default class Style {
  constructor (scope = 'global') {
    this[fields] = {
      themeFile: path.join(__dirname, 'theme.less'),
      scope,
    }
  }

  async compileLess (file) {
    const options = {
      paths: [
        path.dirname(file),
        __dirname
      ],
      plugins: [new LessPlugin(this[fields].themeFile)],
      filename: file,
      compress: true,
      globalVars: { 'node-platform': process.platform },
    }
    const code = await readFile(file, 'utf8')
    return less.render(code, options)
  }

  async applyLess (file) {
    const result = await this.compileLess(file)
    const styleTag = document.createElement('style')
    styleTag.textContent = result.css
    document.querySelector(`#${this[fields].scope}-style`).appendChild(styleTag)
    return new Disposable(() => {
      styleTag.remove()
    })
  }
}
