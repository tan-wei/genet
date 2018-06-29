const m = require('mithril')
const path = require('path')
const { execFile } = require('child_process')
const PermissionMassage = require('./permission-message')
class PcapView {
  constructor () {
    this.devices = []
    this.permission = true
  }

  checkDevices () {
    const cli = path.join(__dirname, 'crates/pcap-cli/target/release/pcap-cli')
    execFile(cli, ['devices'], (error, stdout) => {
      if (error) {
        this.permission = false
      } else {
        this.permission = true
        this.devices = JSON.parse(stdout)
      }
      m.redraw()
    })
  }

  async create (ifs, link) {
    const sess = await genet.session.create()
    sess.regiterStreamReader('pcap', JSON.stringify({
      cmd: './package/pcap/crates/pcap-cli/target/release/pcap_cli',
      args: ['capture', ifs],
      link,
    }))
    sess.startStream()
    sess.on('event', (e) => {
      console.log(e)
    })
    genet.workspace.set('_.pcap.interface', ifs)
    genet.action.emit('core:session:created', sess)
  }

  view (vnode) {
    const ifs = genet.workspace.get('_.pcap.interface')
    this.checkDevices()
    if (!this.permission) {
      return m('div', [
        m(PermissionMassage, {})
      ])
    }
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
              'data-link': dev.link,
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
              const { value, dataset: { link } } =
                ifsElem.options[ifsElem.selectedIndex]
              this.create(value, Number.parseInt(link, 10))
              // Vnode.attrs.callback()
            },
          })
        ])
      ])
    ])
  }
}

module.exports = PcapView
