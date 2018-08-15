import { Disposable } from './disposable'
import fs from 'fs'
import path from 'path'
import { promisify } from 'util'

const readFile = promisify(fs.readFile)
export default class Style {
  private _themeStyle: string
  private _commonStyle: string
  private _scope: string
  constructor(scope: string = 'global') {
    this._themeStyle = fs.readFileSync(path.join(__dirname, 'theme.main.css'), 'utf8')
    this._commonStyle = fs.readFileSync(path.join(__dirname, 'common.main.css'), 'utf8')
    this._scope = scope
  }

  applyTheme(root: DocumentFragment) {
    const styleTag = document.createElement('style')
    styleTag.textContent = this._themeStyle
    const element = root.querySelector('#theme-style')
    if (element) {
      element.appendChild(styleTag)
    }
    return new Disposable(() => {
      styleTag.remove()
    })
  }

  applyCommon(root: DocumentFragment) {
    const styleTag = document.createElement('style')
    styleTag.textContent = this._commonStyle
    const element = root.querySelector('#global-style')
    if (element) {
      element.appendChild(styleTag)
    }
    return new Disposable(() => {
      styleTag.remove()
    })
  }

  async applyCss(root: DocumentFragment, file: string) {
    const styleTag = document.createElement('style')
    styleTag.textContent = await readFile(file, 'utf8')
    const element = root.querySelector(`#${this._scope}-style`)
    if (element) {
      element.appendChild(styleTag)
    }
    return new Disposable(() => {
      styleTag.remove()
    })
  }
}
