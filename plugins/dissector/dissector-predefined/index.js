export default {
  components: [
    {
      type: 'core:attribute',
      attribute: {
        '.src': {
          name: 'Source'
        },
        '.dst': {
          name: 'Destination'
        },
      }
    },
    {
      type: 'core:renderer',
      renderer: {
        type: 'property',
        id: '@flags',
        root: 'flags.js'
      }
    },
    {
      type: 'core:renderer',
      renderer: {
        type: 'property',
        id: '@enum',
        root: 'enum.js'
      }
    },
    {
      type: 'core:renderer',
      renderer: {
        type: 'property',
        id: '@nested',
        root: 'nested.js'
      }
    }
  ]
}
