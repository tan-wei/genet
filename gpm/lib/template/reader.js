const fs = require('fs')
const path = require('path')
module.exports = function template (meta, components, files) {
  components.push({
    type: 'core:library',
    main: 'reader',
  })

  const workspace = files['Cargo.toml'] || {}
  workspace.workspace = (workspace.workspace || { members: [] })
  workspace.workspace.members.push('reader')
  files['Cargo.toml'] = workspace

  files['reader/Cargo.toml'] = {
    package: {
      name: 'reader',
      version: meta.Version,
    },
    lib: {
      'crate-type': [
        'cdylib'
      ],
    },
    dependencies: { 'genet-sdk': '0' },
  }

  files['reader/src/lib.rs'] =
    fs.readFileSync(path.join(__dirname, '../../template/reader.rs'), 'utf8')
}
