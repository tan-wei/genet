require('./include')
const { SessionFactory, Pcap, Chunk } = require('.')

let f = new SessionFactory();
f.networkInterface = 'en0';
f.registerDissector(require('./examples/native').factory)
f.registerStreamDissector(require('./examples/native').streamFactory)
f.options = "{packages: {}}";
f.registerDissector(__dirname + '/dissector.js')
f.registerStreamDissector(__dirname + '/stream_dissector.js')
f.create().then((s) => {
  s.on('status', (stat) => {
    console.log(stat)
  })
  s.on('filter', (filter) => {
    console.log(filter)
    //console.log(s.getFilteredFrames('main', 0, 1))
  })
  s.on('frame', (frame) => {
    console.log(frame)
    console.log(JSON.stringify(s.getFrames(0, 1)[0].rootLayer))
  })
  s.on('log', (log) => {
    console.log(log)
  })
  s.startPcap();
  s.setDisplayFilter('main', 'eth.name == "Ethernet"');
  setTimeout(function(){
    s.destroy()
  }, 2000)
}, (err) => {
  console.log(err)
})
