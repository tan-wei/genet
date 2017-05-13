import fs from 'fs'
import path from 'path'

const importerRegistry = []
const exporterRegistry = []
export default class File {
  static registerImporter (imp) {
    importerRegistry.push(imp)
  }

  static registerExporter (exp) {
    exporterRegistry.push(exp)

    /*
    Const ext = exp.extensions.map((item) => `*.${item}`).join(',')
    Menu.registerHandler(
      {
        path: ['File', 'Export As...', `${exp.name} (${ext})`],
        click: () => GlobalChannel.emit('core:file:export', exp),
      })
    */
  }

  static importers () {
    return importerRegistry
  }

  static exporters () {
    return exporterRegistry
  }

  static async loadFrames (files) {
    const tasks = []
    for (const file of files) {
      const ext = path.extname(file).substr(1)
      for (const imp of File.importers(ext)) {
        const Handler = imp.handler
        const handler = new Handler()
        tasks.push(handler.loadFrames(fs.createReadStream(file)))
      }
    }
    return Promise.all(tasks)
  }
}
