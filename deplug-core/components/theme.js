import Component from './base'
import Theme from '../theme'
import objpath from 'object-path'
import path from 'path'

export default class ThemeComponent extends Component {
  async load () {
    const id = objpath.get(this.comp, 'theme.id', '')
    if (id === '') {
      throw new Error('theme.id field required')
    }

    const name = objpath.get(this.comp, 'theme.name', '')
    if (name === '') {
      throw new Error('theme.name field required')
    }

    const less = objpath.get(this.comp, 'theme.less', '')
    if (less === '') {
      throw new Error('theme.less field required')
    }

    const lessFile = path.join(this.rootDir, less)
    this.theme = new Theme(id, name, lessFile)
    Theme.register(this.theme)
  }

  async unload () {
    Theme.unregister(this.theme)
  }
}
