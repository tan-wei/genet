const cli = require('./cli')
const m = require('mithril')
class MacHelper {
  view () {
    return m('section', [
      m('h1', ['Install Helper Tool']),
      m('p', [
        'macOS does not allow normal users to access pcap devices ' +
        'by default.  Genet provides a tiny helper tool to handle ' +
        'this problem. Once it has been installed, we can start a ' +
        'live capture without running Genet as root.'
      ]),
      m('p', ['Press the following button to install the helper tool.']),
      m('input', {
        type: 'button',
        value: 'Install',
      }),
      m('p', [
        m('a', { href: 'https://genet.app/#/inst?id=helper-tool-on-macos' },
          ['Learn more...'])
      ])
    ])
  }
}

class LinuxHelper {
  view () {
    return m('section', [
      m('h1', ['Not enough capabilities']),
      m('p', [
        'This executable does not have enough capabilities to ' +
        'start a live capture.',
        m('br'),
        'Please apply setcap to the executable.',
        m('p', [
          m('code', ['$ sudo setcap cap_net_raw,cap_net_admin=ep ', cli])
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

class PermissionMassage {
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

module.exports = PermissionMassage
