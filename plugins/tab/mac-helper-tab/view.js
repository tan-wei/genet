import {
  GlobalChannel
} from 'deplug'
import {
  Pcap
} from 'plugkit'
import m from 'mithril'
export default class View {
  view () {
    return m('main', [
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
            if (Pcap.permission) {
              GlobalChannel.emit('core:tab:open', 'Pcap')
            }
          },
        })
      ]),
      m('section', [
        m('h2', ['How it works']),
        m('p', ['The helper tool installer will do:',
          m('ol', [
            m('li', ['Create a new group ‘access_bpf’']),
            m('li', ['Put the current user into the group']),
            m('li', ['Register a startup script'])
          ]), 'The startup script runs the following commands as a ' +
            'previliged user every time:',
          m('ol', [
            m('li', ['chgrp access_bpf /dev/bpf*']),
            m('li', ['chmod g+rw /dev/bpf*'])
          ])
        ])
      ]),
      m('section', [
        m('h2', ['Uninstall the helper tool']),
        m('ol', [
          m('li', ['sudo launchctl unload ' +
            '/Library/LaunchDaemons/net.deplug.DeplugHelper.plist']),
          m('li', ['sudo rm ' +
            '/Library/LaunchDaemons/net.deplug.DeplugHelper.plist']),
          m('li', ['sudo rm ' +
            '/Library/PrivilegedHelperTools/net.deplug.DeplugHelper']),
          m('li', ['sudo dscl . -delete /Groups/access_bpf']),
          m('li', ['sudo chmod g-rw /dev/bpf*'])
        ])
      ])
    ])
  }
}
