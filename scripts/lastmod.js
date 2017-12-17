const path = require('path')
const glob = require('glob')
const fs = require('fs')

module.exports = function (dir) {
  const files = glob.sync(path.resolve(dir, '**/*'), {
    ignore: 'node_modules/**',
    absolute: true,
    cwd: dir
  })
  const stats = files.map((file) => fs.statSync(file))
  return stats.map((stat) => stat.mtime)
    .reduce((max, value) => (typeof max === 'undefined' ||
      max.getTime() < value.getTime()) ? value : max)
      || new Date()
}
