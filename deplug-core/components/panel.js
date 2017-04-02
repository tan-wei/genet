import Component from './base'
import Theme from '../theme'
import jquery from 'jquery'
import objpath from 'object-path'
import path from 'path'

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

    const less = objpath.get(this.comp, 'panel.less', '')
    if (less !== '') {
      const lessFile = path.join(this.rootDir, less)
      const style = await Theme.current.render(lessFile)
      head.append(jquery('<style>').text(style.css))
    }

    const main = objpath.get(this.comp, 'panel.main', '')
    if (main !== '') {
      const mainFile = path.join(this.rootDir, main)
      const func = await this.roll(mainFile)
      func({}, this.rootDir)
    }

    const elements = objpath.get(this.comp, 'panel.elements', [])
    for (const elem of elements) {
      const elemFile = path.join(this.rootDir, elem)
      head.append(jquery(`<link rel="import" href="${elemFile}">`))
    }
  }
}
