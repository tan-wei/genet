import { Disposable } from 'disposables'
import fs from 'fs'
import path from 'path'
import { promisify } from 'util'

const fields = Symbol('fields')
const readFile = promisify(fs.readFile)
export default class Style {
  constructor (scope = 'global') {
    this[fields] = {
      themeStyle: fs.readFileSync(path.join(__dirname, 'theme.main.css'), 'utf8'),
      commonStyle:
        fs.readFileSync(path.join(__dirname, 'common.main.css'), 'utf8'),
      scope,
    }
  }

  applyTheme (root) {
    const styleTag = document.createElement('style')
    styleTag.textContent = this[fields].themeStyle
    root.querySelector('#theme-style').appendChild(styleTag)
    return new Disposable(() => {
      styleTag.remove()
    })
  }

  applyCommon (root) {
    const styleTag = document.createElement('style')
    styleTag.textContent = this[fields].commonStyle
    root.querySelector('#global-style').appendChild(styleTag)
    return new Disposable(() => {
      styleTag.remove()
    })
  }

  async applyCss (root, file) {
    const styleTag = document.createElement('style')
    styleTag.textContent = await readFile(file, 'utf8')
    root.querySelector(`#${this[fields].scope}-style`).appendChild(styleTag)
    return new Disposable(() => {
      styleTag.remove()
    })
  }
}
