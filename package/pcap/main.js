const m = require('mithril')
const path = require('path')
const { execFile } = require('child_process')
class PcapView {
  constructor () {
    this.devices = []
  }
  view (vnode) {
    const ifs = deplug.workspace.get('_.pcap.interface')
    const cli = path.join(__dirname, 'crates/pcap_cli/target/release/pcap_cli')
    execFile(cli, ['devices'], (error, stdout) => {
      this.devices = JSON.parse(stdout)
      m.redraw()
    })
    return m('div', { class: 'livecap-view' }, [
      m('p', [
        m('i', { class: 'fa fa-check' }), ' Live capture is ready.'
      ]),
      m('ul', [
        m('li', [
          m('select', { name: 'ifs' }, this.devices.map((dev) => {
            let { name } = dev
            if (name !== dev.id && process.platform !== 'win32') {
              name += ` - ${dev.id}`
            }
            return m('option', {
              value: dev.id,
              'data-name': name,
              selected: ifs === dev.id,
            }, [name])
          }))
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
