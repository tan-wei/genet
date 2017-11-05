import { Pcap } from 'plugkit'
import m from 'mithril'

export default class PermissionMassage {
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
