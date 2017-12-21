import Deplug from './deplug'
import Style from './style'
import m from 'mithril'
import path from 'path'
import { shell } from 'electron'

export default class Content {
  constructor (view, less, argv = []) {
    this.view = view
    this.less = less
    this.argv = argv
  }

  async load () {
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

    const argv = JSON.parse(decodeURIComponent(location.search.substr(1)))
      .concat(this.argv)
    Reflect.defineProperty(window, 'deplug', { value: new Deplug(argv) })

    const loader = new Style()
    await loader.applyLess(path.join(__dirname, this.less))
    m.mount(document.body, this.view)
    await document.fonts.ready
  }
}
