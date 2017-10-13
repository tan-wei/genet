export default {
  components: [
    {
      type: 'core:dissector',
      dissector: {
        main: 'ethernet.node',
        type: 'packet',
        linkLayers: [
          {link: 1, id: '[eth]'}
        ],
        samples: [
          {
            pcap: 'samples/tcp-ipv4.pcap',
            assert: 'test/eth.json'
          }
        ]
      }
    },
    {
      type: 'core:renderer',
      renderer: {
        type: 'property',
        id: '@eth:mac',
        root: 'mac-addr.js'
      }
    },
    {
      type: 'core:dissector',
      dissector: {
        main: 'ipv4.node',
        type: 'packet',
        samples: [
          { pcap: 'samples/tcp-ipv4.pcap' }
        ]
      }
    },
    {
      type: 'core:filter:transform',
      transform: {
        id: 'ipv4-literal',
        root: 'ipv4-literal.js'
      }
    },
    {
      type: 'core:renderer',
      renderer: {
        type: 'property',
        id: '@ipv4:addr',
        root: 'ipv4-addr.js'
      }
    },
    {
      type: 'core:dissector',
      dissector: {
        main: 'ipv6.node',
        type: 'packet'
      }
    },
    {
      type: 'core:renderer',
      renderer: {
        type: 'property',
        id: '@ipv6:addr',
        root: 'ipv6-addr.js'
      }
    },
    {
      type: 'core:dissector',
      dissector: {
        main: 'udp.node',
        type: 'packet',
        samples: [
          { pcap: 'samples/ntp.pcap' }
        ]
      }
    },
    {
      type: 'core:dissector',
      dissector: {
        main: 'tcp.node',
        type: 'packet'
      }
    },
    {
      type: 'core:renderer',
      renderer: {
        type: 'layer',
        id: 'tcp',
        root: 'tcp-summary.js'
      }
    },
    {
      type: 'core:dissector',
      dissector: {
        type: 'stream',
        main: 'tcp-stream.node'
      }
    },
    {
      type: 'core:dissector',
      dissector: {
        type: 'stream',
        main: 'http.node'
      }
    },
    {
      type: 'core:dissector',
      dissector: {
        type: 'packet',
        main: 'ntp.js'
      }
    }
  ],
  options: [
    {
      id: 'httpPorts',
      name: 'Restrict HTTP Ports',
      type: 'string',
      regexp: '^\\s*([0-9]+,\\s*)*[0-9]*\\s*$',
      default: [80, 8080],
      toJSON: (str) => str.split(',').map((str) => Number.parseInt(str)),
      toString: (json) => json.join(', ')
    }
  ]
}
