import Component from './base'
import Menu from '../menu'
import objpath from 'object-path'
import path from 'path'
import roll from '../roll'

export default class MenuComponent extends Component {
  async load () {
    const menuPath = objpath.get(this.comp, 'menu.path', null)
    if (menuPath === null) {
      throw new Error('menu.path field required')
    }

    const root = objpath.get(this.comp, 'menu.root', '')
    if (root === '') {
      throw new Error('menu.root field required')
    }

    const rootFile = path.join(this.rootDir, root)
    const func = await roll(rootFile, this.rootDir, this.localExtern)
    const module = {}
    func(module)

    const Handler = module.exports
    Menu.registerHandler({
      path: menuPath,
      handler: new Handler(),
      accelerator: objpath.get(this.comp, 'menu.accelerator', ''),
    })
  }
}
