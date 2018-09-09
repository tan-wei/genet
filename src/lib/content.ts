import Genet from './genet'
import Style from './style'
import api from '@genet/api'
import m from 'mithril'
import path from 'path'
const { shell, webFrame } = require('electron')

export default class Content {
  constructor(private view: object, private css: string, private argv: object = {}) {
  }

  async load() {
    webFrame.setVisualZoomLevelLimits(1, 1)

    document.addEventListener('dragover', (event) => {
      event.preventDefault()
      return false
    }, false)

    document.addEventListener('drop', (event) => {
      event.preventDefault()
      return false
    }, false)

    document.addEventListener('click', (event: any) => {
      const isUrl = (/^https?:\/\//).test(event.currentTarget.href)
      if (event.target.tagName === 'A' && isUrl) {
        event.preventDefault()
        shell.openExternal(event.target.href)
        event.preventDefault()
      }
    })

    await new Promise((res) => {
      document.addEventListener('DOMContentLoaded', res)
    })

    const loader = new Style()
    loader.applyTheme(document)
    loader.applyCommon(document)
    await loader.applyCss(document, path.join(__dirname, this.css))

    const argv = Object.assign(
      JSON.parse(decodeURIComponent(location.search.substr(1))), this.argv)
    api.init(new Genet(argv))

    m.mount(document.body, this.view)
    await (document as any).fonts.ready
  }
}
