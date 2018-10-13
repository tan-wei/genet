const fs = require('fs')
const path = require('path')
module.exports = function template (meta, components, files) {
  components.push({
    type: 'core:library',
    main: 'writer',
  })

  const workspace = files['Cargo.toml'] || {}
  workspace.workspace = (workspace.workspace || { members: [] })
  workspace.workspace.members.push('writer')
  files['Cargo.toml'] = workspace

  files['writer/Cargo.toml'] = {
    package: {
      name: 'writer',
      version: meta.Version,
    },
    lib: {
      'crate-type': [
        'cdylib'
      ],
    },
    dependencies: { 'genet-sdk': '0' },
  }

  files['writer/src/lib.rs'] =
    fs.readFileSync(path.join(__dirname, '../../template/writer.rs'), 'utf8')
}
