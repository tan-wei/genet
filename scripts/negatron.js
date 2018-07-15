const jsonfile = require('jsonfile')
const path = require('path')
module.exports = {
  negatronVersion: jsonfile.readFileSync(
    path.join(__dirname, '../package.json')).devDependencies['@genet/negatron'],
  negatronBin: (process.platform === 'win32')
    ? 'node_modules\\.bin\\negatron.cmd'
    : 'node_modules/.bin/negatron',
}
