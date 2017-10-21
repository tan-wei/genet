import Component from './base'
import Renderer from '../renderer'
import objpath from 'object-path'
import path from 'path'
import roll from '../roll'

export default class RendererComponent extends Component {
  async load () {
    const type = objpath.get(this.comp, 'type', '')
    if (type === '') {
      throw new Error('type field required')
    }

    const id = objpath.get(this.comp, 'id', '')
    if (id === '') {
      throw new Error('id field required')
    }

    const root = objpath.get(this.comp, 'root', '')
    if (root === '') {
      throw new Error('root field required')
    }

    const rootFile = path.join(this.rootDir, root)
    const func = await roll(rootFile, this.rootDir, this.localExtern)
    const module = {}
    func(module)

    Renderer.registerProperty(id, module.exports)
  }
}
