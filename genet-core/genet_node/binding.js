const path = require('path')
const modpath = path.join(__dirname, '../bin/genet_node/genet_node.node')
  .replace(/\bapp\.asar\b/, 'app.asar.unpacked')
try {
  module.exports = require(modpath)
} catch (err) {
  module.exports = require('bindings')('genet_node.node')
}
