const importerRegistry = []
const exporterRegistry = []
export default class File {
  static registerImporter (imp) {
    importerRegistry.push(imp)
  }

  static registerExporter (exp) {
    exporterRegistry.push(exp)
  }

  static importers () {
    return importerRegistry
  }

  static exporters () {
    return exporterRegistry
  }
}
