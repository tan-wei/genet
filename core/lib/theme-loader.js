import fs from 'fs'
import less from 'less'
import path from 'path'
import promisify from 'es6-promisify'

const readFile = promisify(fs.readFile)

const fields = Symbol('fields')
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

export default class ThemeLoader {
  constructor (themeFile) {
    this[fields] = { themeFile }
  }

  async load (lessFile, element) {
    const options = {
      paths: [
        path.dirname(lessFile),
        path.join(__dirname, '../../font-awesome/less')
      ],
      plugins: [new LessPlugin(this[fields].themeFile)],
      filename: lessFile,
      compress: true,
      globalVars: { 'node-platform': process.platform },
    }
    const code = await readFile(lessFile, { encoding: 'utf8' })
    const result = await less.render(code, options)

    const styleTag = document.createElement('style')
    styleTag.textContent = result.css
    element.appendChild(styleTag)
    this[fields].styleTag = styleTag
  }
}
