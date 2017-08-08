export default {
  components: [
    {
      type: 'core:panel',
      panel: {
        name: 'Statistics',
        slot: 'core:pcap:bottom',
        less: 'theme.less',
        root: 'view.js'
      }
    }
  ]
}
