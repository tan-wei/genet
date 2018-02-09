import { Pcap } from '@deplug/plugkit'
import PermissionMassage from './permission-message'
import m from 'mithril'

export default class PcapDialog {
  view (vnode) {
    const ifs = deplug.workspace.get('_.pcap.interface')
    if (!Pcap.permission) {
      return m('div', [
        m(PermissionMassage, {})
      ])
    }
    return m('div', [
      m('p', [
        m('i', { class: 'fa fa-check' }), ' Live capture is ready.'
      ]),
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
              deplug.workspace.set('_.pcap.interface', opt.value)
              vnode.attrs.callback(opt.value)
            },
          })
        ])
      ])
    ])
  }
}
