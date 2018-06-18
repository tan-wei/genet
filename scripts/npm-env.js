const path = require('path')
const os = require('os')
const { negatronVersion } = require('./negatron')

module.exports = Object.assign(process.env, {
  npm_config_target: negatronVersion,
  npm_config_devdir: path.resolve(os.homedir(), '.electron-gyp'),
  npm_config_disturl: 'https://atom.io/download/electron',
  npm_config_arch: process.arch,
  npm_config_target_arch: process.arch,
  npm_config_runtime: 'electron',
  npm_config_build_from_source: 'true',
  jobs: '4'
})