import Component from './base'
import Panel from '../panel'
import Theme from '../theme'
import jquery from 'jquery'
import objpath from 'object-path'
import path from 'path'
import roll from '../roll'

export default class PanelComponent extends Component {
  async load () {
    const head = jquery('head')

    const name = objpath.get(this.comp, 'panel.name', '')
    if (name === '') {
      throw new Error('panel.name field required')
    }

    const tag = objpath.get(this.comp, 'panel.tag', '')
    if (tag === '') {
      throw new Error('panel.tag field required')
    }

    const slot = objpath.get(this.comp, 'panel.slot', '')
    if (slot === '') {
      throw new Error('panel.slot field required')
    }

    const less = objpath.get(this.comp, 'panel.less', '')
    if (less !== '') {
      const lessFile = path.join(this.rootDir, less)
      const style = await Theme.current.render(lessFile)
      head.append(jquery('<style>').text(style.css))
    }

    const main = objpath.get(this.comp, 'panel.main', '')
    if (main !== '') {
      const mainFile = path.join(this.rootDir, main)
      const func = await roll(mainFile, this.rootDir, this.localExtern)
      func({}, this.rootDir)
    }

    const root = objpath.get(this.comp, 'panel.root', '')
    if (root === '') {
      throw new Error('panel.root field required')
    }

    const rootFile = path.join(this.rootDir, root)
    const func = await roll(rootFile, this.rootDir, this.localExtern)
    const module = {}
    func(module, this.rootDir)
    Panel.mount(slot, module.exports)
  }
}
