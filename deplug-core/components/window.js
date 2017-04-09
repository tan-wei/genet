import Component from './base'
import Theme from '../theme'
import jquery from 'jquery'
import mithril from 'mithril'
import objpath from 'object-path'
import path from 'path'
import roll from '../roll'
import { remote } from 'electron'

export default class WindowComponent extends Component {
  async load () {
    const head = jquery('head')
    const less = objpath.get(this.comp, 'window.less', '')
    if (less !== '') {
      const lessFile = path.join(this.rootDir, less)
      const style = await Theme.current.render(lessFile)
      head.append(jquery('<style>').text(style.css))
      let computed = getComputedStyle(document.documentElement)
      let vibrancy = JSON.parse(computed.getPropertyValue('--vibrancy'))
      remote.getCurrentWindow().setVibrancy(vibrancy)
    }

    const main = objpath.get(this.comp, 'window.main', '')
    if (main !== '') {
      const mainFile = path.join(this.rootDir, main)
      const func = await roll(mainFile, this.rootDir, this.localExtern)
      func({}, this.rootDir)
    }

    const root = objpath.get(this.comp, 'window.root', '')
    if (root !== '') {
      const rootFile = path.join(this.rootDir, root)
      const func = await roll(rootFile, this.rootDir, this.localExtern)
      const module = {}
      func(module, this.rootDir)
      mithril.mount(document.body, module.exports)
    }
  }
}
