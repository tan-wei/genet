import Component from './base'
import Tab from '../tab'
import objpath from 'object-path'

export default class TabComponent extends Component {
  async load () {
    const name = objpath.get(this.comp, 'tab.name', '')
    if (name === '') {
      throw new Error('tab.name field required')
    }

    const template = objpath.get(this.comp, 'tab.template', '')
    if (template === '') {
      throw new Error('tab.template field required')
    }

    const root = objpath.get(this.comp, 'tab.root', '')
    if (root === '') {
      throw new Error('tab.root field required')
    }

    const tab = Object.assign({ rootDir: this.rootDir }, this.comp.tab)
    Tab.registerTemplate(tab)
  }
}
