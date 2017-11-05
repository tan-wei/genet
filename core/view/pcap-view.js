import { Pcap, SessionFactory } from 'plugkit'
import Dialog from '../lib/dialog'
import m from 'mithril'

class PermissionMassage {
  view () {
    if (Pcap.permission) {
      return m('p', [
        m('i', { class: 'fa fa-check' }), ' Live capture is ready.'
      ])
    }
    switch (process.platform) {
      case 'darwin':
        return m('p', [
          m('i', { class: 'fa fa-exclamation-triangle' }),
          ' Live capture is NOT ready.',
          m('br'),
          'Could not access /dev/bpf*. Please check if the ' +
          'Deplug Helper Tool has been installed correctly.'
        ])
      case 'linux':
        return m('p', [
          m('i', { class: 'fa fa-exclamation-triangle' }),
            ' Live capture is NOT ready.',
            m('br'),
            'The program does not have enough capabilities to ' +
            'start a live capture.',
            m('br'),
            'Please run setcap to the executable and don’t forget ' +
            'to change RPATH.',
          m('ul', [
            m('li', ['$ DEPLUG_BIN=', process.execPath]),
            m('li', ['$ patchelf --set-rpath ' +
              '$(dirname $DEPLUG_BIN) $DEPLUG_BIN']),
            m('li', ['$ sudo setcap cap_net_raw,cap_net_admin=p $DEPLUG_BIN'])
          ])
        ])
      case 'win32':
        return m('p', [
          m('i', { class: 'fa fa-exclamation-triangle' }),
            ' Live capture is NOT ready.', m('br'),
            'Could not load wpcap.dll. Please install WinPcap from ',
            m('a', {
              target: '_blank',
              href: 'https://www.winpcap.org/install/',
            }, ['https://www.winpcap.org/install/']), '.'
        ])
      default:
        return m('p')
    }
  }
}

class PcapDialog {
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

export default class PcapView {
  view () {
    return [
      m('nav', []),
      m('main', [
        m('h1', ['Deplug'])
      ])
    ]
  }

  oncreate () {
    const dialog = new Dialog(PcapDialog)
    dialog.show({ cancelable: false }).then((result) => console.log(result))
  }
}
