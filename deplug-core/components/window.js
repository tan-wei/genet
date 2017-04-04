import Component from './base'
import Theme from '../theme'
import jquery from 'jquery'
import mithril from 'mithril'
import objpath from 'object-path'
import path from 'path'

export default class WindowComponent extends Component {
  async load () {
    const head = jquery('head')
    const less = objpath.get(this.comp, 'window.less', '')
    if (less !== '') {
      const lessFile = path.join(this.rootDir, less)
      const style = await Theme.current.render(lessFile)
      head.append(jquery('<style>').text(style.css))
    }

    const main = objpath.get(this.comp, 'window.main', '')
    if (main !== '') {
      const mainFile = path.join(this.rootDir, main)
      const func = await this.roll(mainFile)
      func({}, this.rootDir)
    }

    const root = objpath.get(this.comp, 'window.root', '')
    if (root !== '') {
      const rootFile = path.join(this.rootDir, root)
      const func = await this.roll(rootFile)
      const module = {}
      func(module, this.rootDir)
      mithril.mount(document.body, module.exports)
    }
  }
}
