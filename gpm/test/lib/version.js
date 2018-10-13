const tempy = require('tempy')
const path = require('path')
const fs = require('fs-extra')
module.exports = () => {
  const dir = tempy.directory()
  const file = path.join(dir, '.version')
  fs.writeJsonSync(file, {
    genet: '0.1.0',
    abi: '0.2',
    target: 'debug',
    resourcePath: dir,
  })

  const builtin = path.join(dir, 'package/pkg/package.json')
  fs.outputJsonSync(builtin, {
    'name': '@genet/pkg',
    'version': '0.1.0',
    'engines': { 'genet': '*' },
    'genet': { 'components': [] },
  })

  const user = path.join(dir, 'user/userpkg/package.json')
  fs.outputJsonSync(user, {
    'name': '@genet/userpkg',
    'version': '0.1.0',
    'engines': { 'genet': '*' },
    'genet': { 'components': [] },
    '_gpm': { source: 'test' },
  })

  return dir
}
