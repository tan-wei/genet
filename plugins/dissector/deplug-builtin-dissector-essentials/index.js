export default {
  components: [
    {
      type: 'dissector',
      dissector: {
        main: 'build/Release/ethernet.node',
        linkLayers: [
          {link: 1, id: '<eth>', name: '<Ethernet>'}
        ],
        samples: [
          { pcap: 'samples/tcp-ipv4.pcap' }
        ]
      }
    },
    {
      type: 'dissector',
      dissector: {
        main: 'build/Release/ipv4.node'
      },
      samples: [
        { pcap: 'samples/tcp-ipv4.pcap' }
      ]
    },
    {
      type: 'dissector',
      dissector: {
        main: 'build/Release/udp.node',
        samples: [
          { pcap: 'samples/ntp.pcap' }
        ]
      }
    },
    {
      type: 'dissector',
      dissector: {
        main: 'build/Release/tcp.node'
      },
      samples: [
        { pcap: 'samples/tcp-ipv4.pcap' }
      ]
    },
    {
      type: 'stream-dissector',
      stream_dissector: {
        main: 'build/Release/tcp-stream.node'
      }
    },
    {
      type: 'stream-dissector',
      stream_dissector: {
        main: 'build/Release/http-stream.node'
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
