import Theme from './theme'
import objpath from 'object-path'
import path from 'path'

export default class Component {
  static create (rootPath, comp) {
    switch (comp.type) {
      case 'theme':
        return new ThemeComponent(rootPath, comp)
      default:
        throw new Error(`unknown component type: ${comp.type}`)
    }
  }

  constructor (rootPath, comp) {
    this.rootPath = rootPath
    this.comp = comp
  }
}

class ThemeComponent extends Component {
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

    const lessFile = path.join(path.dirname(this.rootPath), less)
    Theme.register(new Theme(id, name, lessFile))
  }
}
