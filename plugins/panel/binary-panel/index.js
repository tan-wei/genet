export default {
  components: [
    {
      _type: 'core:panel',
      name: 'Binary',
      slot: 'core:pcap:bottom',
      less: 'theme.less',
      root: 'view.js',
    }
  ],
  reload: 'tab',
  options: {
    layout: {
      title: 'Layout',
      type: 'string',
      enum: ['hex-ascii', 'hex', 'ascii'],
      enum_titles: ['Hex + ASCII', 'Hex', 'ASCII'],
      default: 'hex-ascii',
    }
  },
}
