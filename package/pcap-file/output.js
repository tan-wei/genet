const m = require('mithril')
const { remote: { dialog } } = require('electron')
class OutputView {
  view () {
    return m('ul', [
      m('li', [
        m('input', {
          type: 'button',
          value: 'Export',
          onclick: () => {
            const files = dialog.showSaveDialog({
              properties: ['openFile'],
              filters: [{
                name: 'Pcap File',
                extensions: ['pcap'],
              }],
            })
            if (typeof file !== 'undefined') {

            }
          },
        })
      ])
    ])
  }
}

module.exports = OutputView
