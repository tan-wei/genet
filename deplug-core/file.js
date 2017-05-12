import Menu from './menu'

const importerRegistry = []
const exporterRegistry = []
export default class File {
  static registerImporter (imp) {
    importerRegistry.push(imp)
  }

  static registerExporter (exp) {
    exporterRegistry.push(exp)

    const ext = exp.extensions.map((item) => `*.${item}`).join(',')
    Menu.registerHandler(
      { path: ['File', 'Export As...', `${exp.name} (${ext})`] })
  }

  static importers () {
    return importerRegistry
  }

  static exporters () {
    return exporterRegistry
  }
}
