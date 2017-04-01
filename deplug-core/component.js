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
    const file = objpath.get(this.comp, 'file', null)
    if (file === null) {
      throw new Error('file parameter required')
    }

    const filePath = path.join(path.dirname(this.rootPath), file)
    return filePath
  }
}
