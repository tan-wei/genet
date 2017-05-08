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

File.registerImporter({
  name: 'Pcap File',
  extensions: ['pcap'],
  handler: (readStream) => {

  },
})

File.registerExporter({
  name: 'Pcap File',
  extensions: ['pcap'],
  handler: (writeStream) => {

  },
})
