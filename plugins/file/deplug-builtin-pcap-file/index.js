export default {
  components: [
    {
      type: 'file',
      file: {
        name: 'Pcap',
        extensions: ['.pcap'],
        importer: 'importer.js',
        exporter: 'exporter.js'
      }
    }
  ]
}
