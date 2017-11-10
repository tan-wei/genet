import { Pcap } from 'plugkit'
import m from 'mithril'

class MacHelper {
  view () {
    return m('p', [
      m('section', [
        m('h1', ['A helper tool needs to be installed']),
        m('p', [
          m('b', ['TL;DR:']),
            ' macOS does not allow normal users to access pcap devices ' +
            'by default.  Deplug provides a tiny helper tool to handle ' +
            'this problem. Once it has been installed, we can start a ' +
            'live capture without running Deplug as root.'
        ]),
        m('p', ['Press the follwing button to install the helper tool.']),
        m('input', {
          type: 'button',
          value: Pcap.permission
            ? '✔ Successfully Installed'
            : 'Install',
          disabled: Pcap.permission,
          onclick: () => {
            require('deplug-helper')()
          },
        })
      ]),
      m('p', [
        m('a', { href: 'https://deplug.net/#/helper-tool' }, ['Read more...'])
      ])
    ])
  }
}

export default class PermissionMassage {
  view () {
    switch (process.platform) {
      case 'darwin':
        return m(MacHelper, {})
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
