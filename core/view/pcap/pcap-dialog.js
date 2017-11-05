import { Pcap } from 'plugkit'
import PermissionMassage from './permission-message'
import m from 'mithril'

export default class PcapDialog {
  view (vnode) {
    const ifs = deplug.layout.get('_.pcap.interface')
    return m('div', [
      m(PermissionMassage, {}),
      m('ul', [
        m('li', [
          m('select', { name: 'ifs' }, [
            Pcap.devices.map((dev) => {
              let { name } = dev
              if (name !== dev.id && process.platform !== 'win32') {
                name += ` - ${dev.id}`
              }
              return m('option', {
                value: dev.id,
                'data-name': name,
                selected: ifs === dev.id,
              }, [name])
            })
          ])
        ]),
        m('li', [
          m('input', {
            type: 'button',
            value: 'Start Live Capture',
            onclick: () => {
              const ifsElem = vnode.dom.querySelector('[name=ifs]')
              const opt = ifsElem.options[ifsElem.selectedIndex]
              deplug.layout.set('_.pcap.interface', opt.value)
              vnode.attrs.callback(opt.value)
            },
          })
        ])
      ])
    ])
  }
}
