const nodeExternals = require('webpack-node-externals')
const UglifyJsPlugin = require('uglifyjs-webpack-plugin')
module.exports = {
  target: 'node',
  mode: 'production',
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
  plugins: [
    new UglifyJsPlugin({
      cache: true,
      uglifyOptions: {
        ecma: 8,
        compress: false,
        mangle: true,
      },
    })
  ],
  devtool: 'source-map',
}
