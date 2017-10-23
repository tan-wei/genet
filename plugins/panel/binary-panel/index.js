export default {
  components: [
    {
      _type: 'core:panel',
      name: 'Binary',
      slot: 'core:pcap:bottom',
      less: 'theme.less',
      root: 'view.js'
    }
  ],
  reload: 'tab',
  options: [
    {
      id: 'layout',
      name: 'Layout',
      type: 'enum',
      values: [
        {value: 'hex-ascii', name: 'Hex + ASCII'},
        {value: 'hex', name: 'Hex'},
        {value: 'ascii', name: 'ASCII'}
      ],
      default: 'hex-ascii'
    }
  ]
}
