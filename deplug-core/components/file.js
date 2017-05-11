import Component from './base'
import File from '../file'
import objpath from 'object-path'
import path from 'path'
import roll from '../roll'

export default class FileComponent extends Component {
  async load () {
    const name = objpath.get(this.comp, 'file.name', '')
    if (name === '') {
      throw new Error('file.main field required')
    }

    const extensions = objpath.get(this.comp, 'file.extensions', [])
    if (extensions.length === 0) {
      throw new Error('file.extensions field required')
    }

    const importer = objpath.get(this.comp, 'file.importer', '')
    if (importer !== '') {
      const file = path.join(this.rootDir, importer)
      const func = await roll(file, this.rootDir, this.localExtern)
      const module = {}
      func(module)
      File.registerImporter({
        name,
        extensions,
        handler: module.exports,
      })
    }

    const exporter = objpath.get(this.comp, 'file.exporter', '')
    if (exporter !== '') {
      const file = path.join(this.rootDir, exporter)
      const func = await roll(file, this.rootDir, this.localExtern)
      const module = {}
      func(module)
      File.registerExporter({
        name,
        extensions,
        handler: module.exports,
      })
    }
  }
}
