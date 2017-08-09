import Component from './base'
import Renderer from '../renderer'
import objpath from 'object-path'
import path from 'path'
import roll from '../roll'

export default class PropertyComponent extends Component {
  async load () {
    const type = objpath.get(this.comp, 'property.type', '')
    if (type === '') {
      throw new Error('property.type field required')
    }

    const root = objpath.get(this.comp, 'property.root', '')
    if (root === '') {
      throw new Error('property.root field required')
    }

    const rootFile = path.join(this.rootDir, root)
    const func = await roll(rootFile, this.rootDir, this.localExtern)
    const module = {}
    func(module)
    Renderer.registerProperty(type, module.exports)
  }
}
