export default {
  components: [
    {
      type: 'panel',
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
      id: 'map-resolution',
      name: 'Minimap Resolution',
      type: 'integer',
      min: '10',
      max: '1000',
      default: 100
    }
  ]
}
