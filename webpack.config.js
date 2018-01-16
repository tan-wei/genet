const nodeExternals = require('webpack-node-externals')
module.exports = {
  target: 'node',
  externals: [nodeExternals(), (context, request, callback) => {
    if (request === 'electron') {
      return callback(null, 'commonjs electron')
    }
    callback()
  }],
  cache: true,
  node: {
    __dirname: false,
    __filename: false,
    global: false,
    process: false,
    Buffer: false,
  },
  devtool: 'source-map',
}
