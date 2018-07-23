const cli = require('./cli')
const execa = require('execa')
const m = require('mithril')
class MacHelper {
  view () {
    const url = 'https://github.com/genet-app/genet-helper/releases/' +
      'download/v0.1.0/GenetHelperInstaller.zip'
    return m('section', [
      m('h1', ['Install Helper Tool']),
      m('p', [
        'macOS does not allow normal users to access pcap devices ' +
        'by default.  genet provides a tiny helper tool to handle ' +
        'this problem. Once it has been installed, we can start a ' +
        'live capture without running genet as root.'
      ]),
      m('p', ['Download from here:']),
      m('p', [m('a', { href: url }, [url])]),
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
        ]),
        m('button', {
          onclick: () => {
            this.sudo()
          },
        }, [
          'Set Capabilities'
        ])
      ])
    ])
  }

  sudo () {
    execa.sync('gksudo', [
      '-u', 'root', '-P', '--', 'setcap',
      'cap_net_raw,cap_net_admin=ep',
      cli
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
