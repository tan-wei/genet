const fs = require('fs')
const path = require('path')
module.exports = function template (meta, components, files) {
  components.push({
    type: 'core:panel',
    main: 'panel.js',
    style: 'panel.css',
  })

  files['panel.js'] =
    fs.readFileSync(path.join(__dirname, '../../template/panel.js'), 'utf8')
  files['panel.css'] =
    fs.readFileSync(path.join(__dirname, '../../template/panel.css'), 'utf8')
}
