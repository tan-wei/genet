import Component from './base'
import Menu from '../menu'
import objpath from 'object-path'
import path from 'path'
import roll from '../roll'

export default class MenuComponent extends Component {
  async load () {
    const root = objpath.get(this.comp, 'menu.root', '')
    if (root === '') {
      throw new Error('menu.root field required')
    }
    const rootFile = path.join(this.rootDir, root)
    const func = await roll(rootFile, this.rootDir, this.localExtern)
    func(module)

    for (let menu of module.exports) {
      const menuPath = objpath.get(menu, 'path', null)
      if (menuPath === null) {
        throw new Error('menu.path field required')
      }

      Menu.registerHandler({
        path: menuPath,
        click: menu.click,
        accelerator: menu.accelerator,
        selector: menu.selector,
      })
    }
  }
}
