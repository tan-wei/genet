const path = require('path')
const genet = require('@genet/api')
module.exports = path.join(genet.env.rootPath,
  './package/pcap/crates/pcap-cli/target/release/pcap-cli')
  .replace(/\bapp\.asar\b/, 'app.asar.unpacked')
