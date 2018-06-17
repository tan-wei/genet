const m = require('mithril')
class PcapView {
  view (vnode) {
    return m('div', { class: 'livecap-view' }, [
      m('p', [
        m('i', { class: 'fa fa-check' }), ' Live capture is ready.'
      ]),
      m('ul', [
        m('li', [
          m('select', { name: 'ifs' }, [])
        ]),
        m('li', [
          m('input', {
            type: 'button',
            value: 'Start Live Capture',
            onclick: () => {
              const ifsElem = vnode.dom.querySelector('[name=ifs]')
              const opt = ifsElem.options[ifsElem.selectedIndex]
              deplug.workspace.set('_.pcap.interface', opt.value)
              vnode.attrs.callback(opt.value)
            },
          })
        ])
      ])
    ])
  }
}

module.exports = PcapView
