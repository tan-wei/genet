export default {
  components: [
    {
      _type: 'core:panel',
      name: 'Frames',
      slot: 'core:pcap:top',
      less: 'theme.less',
      root: 'view.js'
    }
  ],
  options: {
    mapResolution: {
      title: 'Minimap Resolution',
      type: 'integer',
      minimum: 10,
      maximum: 1000,
      default: 100
    }
  },
  reload: 'tab'
}
