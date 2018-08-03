import { Disposable } from './disposable'
import fs from 'fs'
import path from 'path'
import { promisify } from 'util'

const fields = Symbol('fields')
const readFile = promisify(fs.readFile)
export default class Style {
  constructor (scope: string = 'global') {
    this[fields] = {
      themeStyle:
        fs.readFileSync(path.join(__dirname, 'theme.main.css'), 'utf8'),
      commonStyle:
        fs.readFileSync(path.join(__dirname, 'common.main.css'), 'utf8'),
      scope,
    }
  }

  applyTheme (root: HTMLDocument) {
    const styleTag = document.createElement('style')
    styleTag.textContent = this[fields].themeStyle
    const element = root.querySelector('#theme-style')
    if (element) {
      element.appendChild(styleTag)
    }
    return new Disposable(() => {
      styleTag.remove()
    })
  }

  applyCommon (root: HTMLDocument) {
    const styleTag = document.createElement('style')
    styleTag.textContent = this[fields].commonStyle
    const element = root.querySelector('#global-style')
    if (element) {
      element.appendChild(styleTag)
    }
    return new Disposable(() => {
      styleTag.remove()
    })
  }

  async applyCss (root: HTMLDocument, file: string) {
    const styleTag = document.createElement('style')
    styleTag.textContent = await readFile(file, 'utf8')
    const element = root.querySelector(`#${this[fields].scope}-style`)
    if (element) {
      element.appendChild(styleTag)
    }
    return new Disposable(() => {
      styleTag.remove()
    })
  }
}
