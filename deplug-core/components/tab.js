import Component from './base'
import Tab from '../tab'
import objpath from 'object-path'

export default class TabComponent extends Component {
  async load () {
    const name = objpath.get(this.comp, 'name', '')
    if (name === '') {
      throw new Error('name field required')
    }

    const template = objpath.get(this.comp, 'template', '')
    if (template === '') {
      throw new Error('template field required')
    }

    const root = objpath.get(this.comp, 'root', '')
    if (root === '') {
      throw new Error('root field required')
    }

    const tab = Object.assign({ rootDir: this.rootDir }, this.comp)
    Tab.registerTemplate(tab)
  }
}
