export default {
  components: [
    {
      _type: 'core:panel',
      name: 'Layers',
      slot: 'core:pcap:middle',
      less: 'theme.less',
      root: 'view.js'
    }
  ],
  options: {
    tsformat: {
      title: 'Timestamp Format',
      type: 'string',
      pattern: '^.+$',
      default: 'YYYY-MM-DDTHH:mm:ss.SSSZ'
    }
  },
  reload: 'tab'
}
