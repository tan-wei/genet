const m = require('mithril')
class OutputView {
  view (vnode) {
    return m('div', [
      `${vnode.attrs.getFilter()}`
    ])
  }
}

module.exports = OutputView
