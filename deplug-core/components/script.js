import Component from './base'
import objpath from 'object-path'
import path from 'path'
import roll from '../roll'

export default class ScriptComponent extends Component {
  async load () {
    const main = objpath.get(this.comp, 'script.main', '')
    if (main === '') {
      throw new Error('script.main field required')
    }

    const mainFile = path.join(this.rootDir, main)
    const func = await roll(mainFile, this.rootDir, this.localExtern)
    const module = {}
    func(module)
  }
}
