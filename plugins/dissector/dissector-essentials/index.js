export default {
  components: [
    {
      _type: 'core:dissector',
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
    },
    {
      _type: 'core:renderer',
      type: 'property',
      id: '@eth:mac',
      root: 'mac-addr.js'
    },
    {
      _type: 'core:dissector',
      main: 'ipv4.node',
      type: 'packet',
      samples: [
        { pcap: 'samples/tcp-ipv4.pcap' }
      ]
    },
    {
      _type: 'core:filter:transform',
      id: 'ipv4-literal',
      root: 'ipv4-literal.js'
    },
    {
      _type: 'core:renderer',
      type: 'property',
      id: '@ipv4:addr',
      root: 'ipv4-addr.js'
    },
    {
      _type: 'core:dissector',
      main: 'ipv6.node',
      type: 'packet'
    },
    {
      _type: 'core:renderer',
      type: 'property',
      id: '@ipv6:addr',
      root: 'ipv6-addr.js'
    },
    {
      _type: 'core:dissector',
      main: 'udp.node',
      type: 'packet',
      samples: [
        { pcap: 'samples/ntp.pcap' }
      ]
    },
    {
      _type: 'core:dissector',
      main: 'tcp.node',
      type: 'packet'
    },
    {
      _type: 'core:renderer',
      type: 'layer',
      id: 'tcp',
      root: 'tcp-summary.js'
    },
    {
      _type: 'core:dissector',
      type: 'stream',
      main: 'tcp-stream.node'
    },
    {
      _type: 'core:dissector',
      type: 'stream',
      main: 'http.node'
    },
    {
      _type: 'core:dissector',
      type: 'packet',
      main: 'ntp.js'
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
  ],
  reload: 'tab'
}
