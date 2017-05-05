import Component from './base'
import GlobalChannel from '../global-channel'
import Theme from '../theme'
import mithril from 'mithril'
import objpath from 'object-path'
import path from 'path'
import { remote } from 'electron'
import roll from '../roll'

export default class WindowComponent extends Component {
  async load () {
    this.styleTag = document.createElement('style')
    document.head.appendChild(this.styleTag)

    this.updateTheme = async () => {
      if (this.lessFile) {
        const style = await Theme.current.render(this.lessFile)
        this.styleTag.textContent = style.css
        const computed = getComputedStyle(document.documentElement)
        const vibrancy = JSON.parse(computed.getPropertyValue('--vibrancy'))
        remote.getCurrentWindow().setVibrancy(vibrancy)
      }
    }
    GlobalChannel.on('core:theme:updated', this.updateTheme)

    const less = objpath.get(this.comp, 'window.less', '')
    if (less !== '') {
      this.lessFile = path.join(this.rootDir, less)
      await this.updateTheme()
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
      func(module)
      mithril.mount(document.body, module.exports)
    }
  }

  async unload () {
    GlobalChannel.removeListener('core:theme:updated', this.updateTheme)
    this.styleTag.remove()
  }
}
