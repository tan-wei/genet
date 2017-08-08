export default {
  components: [
    {
      type: 'core:panel',
      panel: {
        name: 'Frames',
        slot: 'core:pcap:top',
        less: 'theme.less',
        root: 'view.js'
      }
    }
  ],
  options: [
    {
      id: 'mapResolution',
      name: 'Minimap Resolution',
      type: 'integer',
      min: 10,
      max: 1000,
      default: 100
    }
  ]
}
