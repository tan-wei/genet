export default {
  components: [
    {
      type: 'panel',
      panel: {
        name: 'Layers',
        slot: 'core:pcap:middle',
        less: 'theme.less',
        root: 'view.js'
      }
    }
  ],
  options: [
    {
      id: 'tsformat',
      name: 'Timestamp Format',
      type: 'string',
      regexp: '^.+$',
      default: 'YYYY-MM-DDTHH:mm:ss.SSSZ'
    }
  ]
}
