const path = require('path')
const genet = require('@genet/api')
const target = process.env.GENET_TARGET
module.exports = path.join(genet.env.rootPath,
  `./package/pcap/crates/pcap-cli/target/${target}/pcap-cli`)
  .replace(/\bapp\.asar\b/, 'app.asar.unpacked')
