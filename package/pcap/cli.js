const path = require('path')
module.exports = path.join(genet.env.rootPath,
  './package/pcap/crates/pcap-cli/target/release/pcap-cli')
  .replace(/\bapp\.asar\b/, 'app.asar.unpacked')
