export default {
  components: [
    {
      type: 'core:file',
      file: {
        name: 'Pcap',
        extensions: ['pcap'],
        importer: 'importer.js',
        exporter: 'exporter.js'
      }
    }
  ]
}
