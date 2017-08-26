export default {
  components: [
    {
      type: 'core:dissector',
      dissector: {
        main: 'build/Release/ethernet.node',
        linkLayers: [
          {link: 1, id: '[eth]'}
        ],
        samples: [
          {
            pcap: 'samples/tcp-ipv4.pcap',
            assert: 'test/eth.json'
          }
        ],
        descriptors: {
          eth: {
            name: 'Ethernet'
          },
          '.src': {
            name: 'Source'
          },
          '.dst': {
            name: 'Destination'
          },
          'eth.type': {
            name: 'EtherType'
          },
          'eth.type.ipv4': {
            name: 'IPv4'
          },
          'eth.type.ipv6': {
            name: 'IPv6'
          }
        }
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
        main: 'build/Release/ipv4.node',
        samples: [
          { pcap: 'samples/tcp-ipv4.pcap' }
        ],
        descriptors: {
          ipv4: {
            name: 'IPv4'
          },
          'ipv4.version': {
            name: 'Version'
          },
          'ipv4.hLen': {
            name: 'Internet Header Length'
          },
          'ipv4.type': {
            name: 'Type Of Service'
          },
          'ipv4.tLen': {
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
          'ipv4.flags.dontFrag': {
            name: 'Don\'t Fragment'
          },
          'ipv4.flags.moreFrag': {
            name: 'More Fragments'
          },
          'ipv4.fOffset': {
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
            name: 'Source'
          },
          'ipv4.dst': {
            name: 'Destination'
          }
        }
      }
    },
    {
      type: 'core:filter:literal',
      renderer: {
        id: 'ipv4',
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
    },
    {
      type: 'core:dissector',
      dissector: {
        main: 'build/Release/ipv6.node',
        descriptors: {
          ipv6: {
            name: 'IPv6'
          },
          'ipv6.version': {
            name: 'Version'
          },
          'ipv6.tClass': {
            name: 'Traffic Class'
          },
          'ipv6.fLevel': {
            name: 'Flow Level'
          },
          'ipv6.pLen': {
            name: 'Payload Length'
          },
          'ipv6.nHeader': {
            name: 'Next Header'
          },
          'ipv6.hLimit': {
            name: 'Hop Limit'
          },
          'ipv6.src': {
            name: 'Source'
          },
          'ipv6.dst': {
            name: 'Destination'
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
          }
        }
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
        main: 'build/Release/udp.node',
        samples: [
          { pcap: 'samples/ntp.pcap' }
        ],
        descriptors: {
          udp: {
            name: 'UDP'
          },
          'udp.src': {
            name: 'Source'
          },
          'udp.dst': {
            name: 'Destination'
          },
          'udp.length': {
            name: 'Length'
          },
          'udp.checksum': {
            name: 'Checksum'
          }
        }
      }
    },
    {
      type: 'core:dissector',
      dissector: {
        main: 'build/Release/tcp.node',
        descriptors: {
          tcp: {
            name: 'TCP'
          },
          'tcp.src': {
            name: 'Source'
          },
          'tcp.dst': {
            name: 'Destination'
          },
          'tcp.seq': {
            name: 'Sequence number'
          },
          'tcp.ack': {
            name: 'Acknowledgment number'
          },
          'tcp.dOffset': {
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
          'tcp.options.selAck': {
            name: 'Selective ACK'
          },
          'tcp.options.nop': {
            name: 'NOP'
          },
          'tcp.options.ts': {
            name: 'Timestamps'
          },
          'tcp.options.ts.mt': {
            name: 'My timestamp'
          },
          'tcp.options.ts.et': {
            name: 'Echo reply timestamp'
          }
        }
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
        main: 'build/Release/tcp-stream.node',
        descriptors: {
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
          }
        }
      }
    },
    {
      type: 'core:dissector',
      dissector: {
        main: 'build/Release/http.node'
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
