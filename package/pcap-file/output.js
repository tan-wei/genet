const m = require('mithril')
const { remote: { dialog } } = require('electron')
class OutputView {
  view (vnode) {
    return m('ul', [
      m('li', [
        m('input', {
          type: 'button',
          value: 'Export',
          onclick: () => {
            const file = dialog.showSaveDialog({
              properties: ['openFile'],
              filters: [{
                name: 'Pcap File',
                extensions: ['pcap'],
              }],
            })
            if (typeof file !== 'undefined') {
              vnode.attrs.callback('pcap-file', { file })
            }
          },
        })
      ])
    ])
  }
}

module.exports = OutputView
