export default {
  components: [
    {
      type: 'dissector',
      dissector: {
        main: 'build/Release/ethernet.node',
        linkLyaers: [
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
    }
  ]
}
