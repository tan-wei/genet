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
          { pcap: 'samples/eth.pcap' }
        ]
      }
    },
    {
      type: 'dissector',
      dissector: {
        main: 'build/Release/ipv4.node'
      }
    },
    {
      type: 'dissector',
      dissector: {
        main: 'build/Release/tcp.node'
      }
    },
    {
      type: 'stream-dissector',
      stream_dissector: {
        main: 'build/Release/tcp-stream.node'
      }
    }
  ]
}
