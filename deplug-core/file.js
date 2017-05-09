const importers = []
const exporters = []
export default class File {
  static registerImporter (imp) {
    importers.push(imp)
  }

  static registerExporter (exp) {
    exporters.push(exp)
  }
}
