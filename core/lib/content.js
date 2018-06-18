import { shell, webFrame } from 'electron'
import Genet from './genet'
import Style from './style'
import m from 'mithril'
import path from 'path'

export default class Content {
  constructor (view, css, argv = []) {
    this.view = view
    this.css = css
    this.argv = argv
  }

  async load () {
    webFrame.setZoomLevelLimits(1, 1)

    document.addEventListener('dragover', (event) => {
      event.preventDefault()
      return false
    }, false)

    document.addEventListener('drop', (event) => {
      event.preventDefault()
      return false
    }, false)

    document.addEventListener('click', (event) => {
      const isUrl = (/^https?:\/\//).test(event.target.href)
      if (event.target.tagName === 'A' && isUrl) {
        event.preventDefault()
        shell.openExternal(event.target.href)
      }
    })

    await new Promise((res) => {
      document.addEventListener('DOMContentLoaded', res)
    })

    const loader = new Style()
    loader.applyTheme(document)
    loader.applyCommon(document)
    await loader.applyCss(document, path.join(__dirname, this.css))

    const argv = JSON.parse(decodeURIComponent(location.search.substr(1)))
      .concat(this.argv)
    Reflect.defineProperty(window, 'genet', { value: new Genet(argv) })

    m.mount(document.body, this.view)
    await document.fonts.ready
  }
}
