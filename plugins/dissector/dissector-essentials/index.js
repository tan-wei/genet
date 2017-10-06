export default {
  components: [
    {
      type: 'core:attribute',
      attribute: {
        eth: {
          name: 'Ethernet'
        },
        'eth.type': {
          name: 'EtherType'
        },
        'eth.type.ipv4': {
          name: 'IPv4'
        },
        'eth.type.ipv6': {
          name: 'IPv6'
        },
        ipv4: {
          name: 'IPv4'
        },
        'ipv4.version': {
          name: 'Version'
        },
        'ipv4.headerLength': {
          name: 'Internet Header Length'
        },
        'ipv4.type': {
          name: 'Type Of Service'
        },
        'ipv4.totalLength': {
          name: 'Total Length'
        },
        'ipv4.id': {
          name: 'Identification'
        },
        'ipv4.flags': {
          name: 'Flags'
        },
        'ipv4.flags.reserved': {
          name: 'Reserved'
        },
        'ipv4.flags.dontFragment': {
          name: 'Don\'t Fragment'
        },
        'ipv4.flags.moreFragments': {
          name: 'More Fragments'
        },
        'ipv4.fragmentOffset': {
          name: 'Fragment Offset'
        },
        'ipv4.ttl': {
          name: 'TTL'
        },
        'ipv4.protocol': {
          name: 'Protocol'
        },
        'ipv4.protocol.icmp': {
          name: 'ICMP'
        },
        'ipv4.protocol.igmp': {
          name: 'IGMP'
        },
        'ipv4.protocol.tcp': {
          name: 'TCP'
        },
        'ipv4.protocol.udp': {
          name: 'UDP'
        },
        'ipv4.checksum': {
          name: 'Header Checksum'
        },
        'ipv4.src': {
          name: 'Source',
          virtual: true
        },
        'ipv4.dst': {
          name: 'Destination',
          virtual: true
        },
        ipv6: {
          name: 'IPv6'
        },
        'ipv6.version': {
          name: 'Version'
        },
        'ipv6.trafficClass': {
          name: 'Traffic Class'
        },
        'ipv6.flowLevel': {
          name: 'Flow Level'
        },
        'ipv6.payloadLength': {
          name: 'Payload Length'
        },
        'ipv6.nextHeader': {
          name: 'Next Header'
        },
        'ipv6.hopLimit': {
          name: 'Hop Limit'
        },
        'ipv6.hopByHop': {
          name: 'Hop-by-Hop Options'
        },
        'ipv6.src': {
          name: 'Source',
          virtual: true
        },
        'ipv6.dst': {
          name: 'Destination',
          virtual: true
        },
        'ipv6.protocol': {
          name: 'Protocol'
        },
        'ipv6.protocol.icmp': {
          name: 'ICMP'
        },
        'ipv6.protocol.igmp': {
          name: 'IGMP'
        },
        'ipv6.protocol.tcp': {
          name: 'TCP'
        },
        'ipv6.protocol.udp': {
          name: 'UDP'
        },
        udp: {
          name: 'UDP'
        },
        'udp.src': {
          name: 'Source',
          virtual: true
        },
        'udp.dst': {
          name: 'Destination',
          virtual: true
        },
        'udp.length': {
          name: 'Length'
        },
        'udp.checksum': {
          name: 'Checksum'
        },
        tcp: {
          name: 'TCP'
        },
        'tcp.src': {
          name: 'Source',
          virtual: true
        },
        'tcp.dst': {
          name: 'Destination',
          virtual: true
        },
        'tcp.streamId': {
          name: 'Stream ID'
        },
        'tcp.seq': {
          name: 'Sequence number'
        },
        'tcp.ack': {
          name: 'Acknowledgment number'
        },
        'tcp.dataOffset': {
          name: 'Data offset'
        },
        'tcp.flags': {
          name: 'Flags'
        },
        'tcp.flags.ns': {
          name: 'NS'
        },
        'tcp.flags.cwr': {
          name: 'CWR'
        },
        'tcp.flags.ece': {
          name: 'ECE'
        },
        'tcp.flags.urg': {
          name: 'URG'
        },
        'tcp.flags.ack': {
          name: 'ACK'
        },
        'tcp.flags.psh': {
          name: 'PSH'
        },
        'tcp.flags.rst': {
          name: 'RST'
        },
        'tcp.flags.syn': {
          name: 'SYN'
        },
        'tcp.flags.fin': {
          name: 'FIN'
        },
        'tcp.window': {
          name: 'Window size'
        },
        'tcp.checksum': {
          name: 'Checksum'
        },
        'tcp.urgent': {
          name: 'Urgent pointer'
        },
        'tcp.options': {
          name: 'Options'
        },
        'tcp.options.selectiveAck': {
          name: 'Selective ACK'
        },
        'tcp.options.nop': {
          name: 'NOP'
        },
        'tcp.options.ts': {
          name: 'Timestamps'
        },
        'tcp.options.ts.my': {
          name: 'My timestamp'
        },
        'tcp.options.ts.echo': {
          name: 'Echo reply timestamp'
        },
        'tcp.stream': {
          name: 'Stream'
        },
        'tcp.stream.length': {
          name: 'Total Received Length'
        },
        'tcp.stream.payloads': {
          name: 'Reassembled Payloads'
        },
        'tcp.stream.lastSeq': {
          name: 'Last Sequence Number'
        },
        http: {
          name: 'Hypertext Transfer Protocol'
        },
        'http.headers': {
          name: 'Headers'
        }
      }
    },
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
