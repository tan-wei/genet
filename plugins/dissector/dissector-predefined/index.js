export default {
  components: [
    {
      _type: 'core:renderer',
      type: 'property',
      id: '@flags',
      root: 'flags.js'
    },
    {
      _type: 'core:renderer',
      type: 'property',
      id: '@enum',
      root: 'enum.js'
    },
    {
      _type: 'core:renderer',
      type: 'property',
      id: '@nested',
      root: 'nested.js'
    }
  ],
  reload: 'tab'
}
