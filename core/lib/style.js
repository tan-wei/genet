import { Disposable } from 'disposables'
import fs from 'fs'
import less from 'less'
import path from 'path'
import { promisify } from 'util'

const fields = Symbol('fields')
const readFile = promisify(fs.readFile)
export default class Style {
  constructor (scope = 'global') {
    this[fields] = {
      themeFile: fs.readFileSync(path.join(__dirname, 'theme.css'), 'utf8'),
      scope,
    }
  }

  async compileLess (file) {
    const options = {
      paths: [
        path.dirname(file),
        __dirname
      ],
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
    const theme = this[fields].themeFile
    styleTag.textContent = `${theme}\n${result.css}`
    document.querySelector(`#${this[fields].scope}-style`).appendChild(styleTag)
    return new Disposable(() => {
      styleTag.remove()
    })
  }
}
