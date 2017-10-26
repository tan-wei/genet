export default {
  components: [
    {
      _type: 'core:panel',
      name: 'Frames',
      slot: 'core:pcap:top',
      less: 'theme.less',
      root: 'view.js',
    }
  ],
  options: {
    mapResolution: {
      title: 'Minimap Resolution',
      description: 'Higher value may cause a performance issue',
      type: 'integer',
      minimum: 10,
      maximum: 1000,
      default: 100,
    },
  },
  reload: 'tab',
}
