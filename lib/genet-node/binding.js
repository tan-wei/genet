const path = require('path')
const modpath = path.join(__dirname, '../bin/genet-node/genet-node.node')
  .replace(/\bapp\.asar\b/, 'app.asar.unpacked')
try {
  module.exports = require(modpath)
} catch (err) {
  module.exports = require('bindings')('genet-node.node')
}
