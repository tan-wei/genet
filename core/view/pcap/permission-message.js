import { Pcap } from '@deplug/plugkit'
import m from 'mithril'

class MacHelper {
  view () {
    return m('section', [
      m('h1', ['Install Helper Tool']),
      m('p', [
        'macOS does not allow normal users to access pcap devices ' +
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
          require('@deplug/osx-helper').install()
          if (Pcap.permission) {
            deplug.notify.show(
              'Successfully installed.', {
                type: 'success',
                title: 'Helper Tool',
              })
          } else {
            deplug.notify.show(
              'Installation failed.', {
                type: 'error',
                title: 'Helper Tool',
              })
          }
        },
      }),
      m('p', [
        m('a', { href: 'https://deplug.net/#/helper-tool' }, ['Learn more...'])
      ])
    ])
  }
}

class LinuxHelper {
  view () {
    return m('section', [
      m('h1', ['Not enougth capabilities']),
      m('p', [
        'This executable does not have enough capabilities to ' +
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
    ])
  }
}

class WinHelper {
  view () {
    return m('section', [
      m('h1', ['WinPcap Required']),
      m('p', [
        'Could not load wpcap.dll. Please install WinPcap from ',
        m('a', {
          target: '_blank',
          href: 'https://www.winpcap.org/install/',
        }, ['https://www.winpcap.org/install/']), '.'
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
      return m(LinuxHelper, {})
      case 'win32':
      return m(WinHelper, {})
      default:
      return m('p')
    }
  }
}
