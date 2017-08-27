import Component from './base'
import Session from '../session'
import objpath from 'object-path'
import path from 'path'
import roll from '../roll'

export default class literalComponent extends Component {
  async load () {
    const id = objpath.get(this.comp, 'literal.id', '')
    if (id === '') {
      throw new Error('literal.id field required')
    }

    const root = objpath.get(this.comp, 'literal.root', '')
    if (root === '') {
      throw new Error('literal.root field required')
    }

    const regexp = objpath.get(this.comp, 'literal.regexp')

    const rootFile = path.join(this.rootDir, root)
    const func = await roll(rootFile, this.rootDir, this.localExtern)
    const module = {}
    func(module)

    Session.registerFilterLiteral({
      id,
      regexp,
      parse: module.exports,
    })
  }
}
