import { Disposable } from 'disposables'
import fs from 'fs'
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

  get themeFile () {
    return this[fields].themeFile
  }

  async applyLess (file) {
    const css = await readFile(file, 'utf8')
    const styleTag = document.createElement('style')
    const theme = this[fields].themeFile
    styleTag.textContent = `${theme}\n${css}`
    document.querySelector(`#${this[fields].scope}-style`).appendChild(styleTag)
    return new Disposable(() => {
      styleTag.remove()
    })
  }
}
