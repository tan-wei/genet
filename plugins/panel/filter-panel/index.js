export default {
  components: [
    {
      type: 'core:panel',
      panel: {
        name: 'Filter',
        slot: 'core:pcap:tool',
        less: 'theme.less',
        root: 'view.js'
      }
    }
  ]
}
