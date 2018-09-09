const fs = require('fs-extra')
const path = require('path')
module.exports = {
  negatronVersion: fs.readJsonSync(
    path.join(__dirname, '../package.json')).devDependencies['electron'],
  negatronBin: (process.platform === 'win32')
    ? 'node_modules\\.bin\\electron.cmd'
    : 'node_modules/.bin/electron',
}
