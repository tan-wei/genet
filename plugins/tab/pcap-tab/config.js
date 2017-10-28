import {
  File,
  PluginLoader,
  Profile,
  Session,
  Tab,
  Channel,
  GlobalChannel
} from 'deplug'
import { Pcap, SessionFactory } from 'plugkit'
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

function prepareSession () {
  const factory = new SessionFactory()
  factory.options = Profile.current.object
  factory.networkInterface = Tab.options.ifs || ''
  for (const layer of Session.linkLayers) {
    factory.registerLinkLayer(layer)
  }
  for (const diss of Session.dissectors) {
    factory.registerDissector(diss)
  }
  for (const trans of Session.filterTransforms) {
    factory.registerFilterTransform(trans)
  }
  factory.registerAttributes(Session.attributes)
  return factory
}
export default class ConfigView {
  constructor () {
    this.loaded = false
    this.load()
  }
  async load () {
    await PluginLoader.loadComponents('core:dissector')
    this.loaded = true
    m.redraw()
  }
  startPcap (vnode) {
    const ifsSelector = vnode.dom.querySelector('[name=ifs]')
    const opt = ifsSelector.options[ifsSelector.selectedIndex]
    const ifs = opt.value
    const ifsName = opt.getAttribute('data-name')
    Object.assign(Tab.options, {
      ifs,
      ifsName,
    })
    Tab.page = 'pcap'
    setTimeout(() => {
      const factory = prepareSession()
      factory.snaplen = Profile.current.get('_.snaplen')
      factory.create().then((sess) => {
        if (Tab.options.ifs) {
          sess.startPcap()
        }
        Channel.emit('core:pcap:session-created', sess)
      }, (err) => {
        // eslint-disable-next-line no-console
        console.log(err)
      })
      GlobalChannel.emit(
        'core:tab:set-name', Tab.id, `${Tab.options.ifsName} @ Live Capture`)
    }, 100)
  }
  oncreate () {
    if (Tab.options.files) {
      Tab.page = 'pcap'
      m.redraw()
      setTimeout(() => {
        File.loadFrames(Tab.options.files).then((results) => {
          const factory = prepareSession()
          factory.create().then((sess) => {
            Channel.emit('core:pcap:session-created', sess)
            for (const pcap of results) {
              sess.analyze(pcap.frames.map((frame) => ({
                link: pcap.link,
                payload: frame.payload,
                length: frame.length,
                timestamp: frame.timestamp,
                sourceId: 0,
              })))
            }
          })
        })
      }, 100)
    }
  }
  view (vnode) {
    return m('main', [
      m('section', [
        m('h1', ['Live capture']),
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
                }, [name])
              })
            ])
          ]),
          m('li', [
            m('input', {
              type: 'button',
              value: 'Start Live Capture',
              disabled: !this.loaded,
              onclick: () => {
                this.startPcap(vnode)
              },
            })
          ])
        ])
      ])
    ])
  }
}
