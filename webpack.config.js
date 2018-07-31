const nodeExternals = require('webpack-node-externals')
const mode = process.env.NODE_ENV === 'production'
  ? 'production'
  : 'development'
module.exports = {
  target: 'node',
  mode,
  externals: [nodeExternals(), (context, request, callback) => {
    if (request === 'electron') {
      return callback(null, 'commonjs electron')
    }
    callback()
  }],
  module: {
    rules: [
      {
        test: /\.ts$/,
        use: [
          {
            loader: 'ts-loader',
            options: { transpileOnly: true },
          }
        ],
      }
    ],
  },
  entry: {
    app: './src/app.main.js',
    render: './src/render.main.js',
  },
  output: {
    filename: '[name].main.js',
    path: `${__dirname}/genet_modules/src`,
  },
  cache: true,
  node: {
    __dirname: false,
    __filename: false,
    global: false,
    process: false,
    Buffer: false,
  },
  resolve: { extensions: ['.ts', '.js'] },
  devtool: 'source-map',
}
