import Component from './base'
import Tab from '../tab'
import Theme from '../theme'
import jquery from 'jquery'
import objpath from 'object-path'
import path from 'path'

export default class TabComponent extends Component {
  async load () {
    const head = jquery('head')

    const name = objpath.get(this.comp, 'tab.name', '')
    if (name === '') {
      throw new Error('tab.name field required')
    }

    const less = objpath.get(this.comp, 'tab.less', '')
    if (less !== '') {
      const lessFile = path.join(this.rootDir, less)
      const style = await Theme.current.render(lessFile)
      head.append(jquery('<style>').text(style.css))
    }

    const root = objpath.get(this.comp, 'tab.root', '')
    if (root === '') {
      throw new Error('tab.root field required')
    }

    Tab.registerTemplate(this.comp.tab)
  }
}
