const m = require('mithril')
class FilterInspectorView {
  constructor () {
    this.displayFilter = null
  }

  oncreate () {
    deplug.action.on('core:filter:updated', (filter) => {
      this.displayFilter = filter
      m.redraw()
    })
  }

  view () {
    if (this.displayFilter === null) {
      return m('div')
    }
    return m('div', { class: 'view' }, [
      m('pre', [
        this.displayFilter.transpiled.globals.join('\n')
      ]),
      m('pre', [this.displayFilter.transpiled.expression])
    ])
  }
}

module.exports = FilterInspectorView
