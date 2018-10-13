const fs = require('fs')
const path = require('path')
module.exports = function template (meta, components, files) {
  components.push({
    type: 'core:library',
    main: 'decoder',
  })

  const workspace = files['Cargo.toml'] || {}
  workspace.workspace = (workspace.workspace || { members: [] })
  workspace.workspace.members.push('decoder')
  files['Cargo.toml'] = workspace

  files['decoder/Cargo.toml'] = {
    package: {
      name: 'decoder',
      version: meta.Version,
    },
    lib: {
      'crate-type': [
        'cdylib'
      ],
    },
    dependencies: { 'genet-sdk': '0' },
  }

  files['decoder/src/lib.rs'] =
    fs.readFileSync(path.join(__dirname, '../../template/decoder.rs'), 'utf8')
}
