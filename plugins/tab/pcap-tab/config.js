import { File, PluginLoader, Profile, Session, Tab, Channel, GlobalChannel } from 'deplug'
import { Pcap, SessionFactory } from 'plugkit'
import m from 'mithril'

class PermissionMassage {
  view(vnode) {
    if (Pcap.permission) {
      return <p><i class="fa fa-check"></i> Live capture is ready.</p>
    }
    switch (process.platform) {
      case 'darwin':
        return <p>
        <i class="fa fa-exclamation-triangle"></i> Live capture is NOT ready.<br></br>
          Could not access /dev/bpf*.
          Please check if the Deplug Helper Tool has been installed correctly.
        </p>
      case 'linux':
        return <p>
        <i class="fa fa-exclamation-triangle"></i> Live capture is NOT ready.<br></br>
        The program does not have enough capabilities to start a live capture.<br></br>
        Please run setcap to the executable and don’t forget to change RPATH.
        <ul>
          <li>$ DEPLAG_BIN={process.execPath}</li>
          <li>$ patchelf --set-rpath $(dirname $DEPLAG_BIN) $DEPLAG_BIN</li>
          <li>$ sudo setcap cap_net_raw,cap_net_admin=p $DEPLAG_BIN</li>
        </ul>
        </p>
      case 'win32':
        return <p>
        <i class="fa fa-exclamation-triangle"></i> Live capture is NOT ready.<br></br>
        Could not load wpcap.dll.
        Please install WinPcap from <a target="_blank" href="https://www.winpcap.org/install/">
          https://www.winpcap.org/install/</a>.
        </p>
    }
    return <p></p>
  }
}

export default class ConfigView {
  constructor() {
    this.loaded = false
    this.load()
  }

  async load() {
    await PluginLoader.loadComponents('dissector')
    await PluginLoader.loadComponents('stream-dissector')
    this.loaded = true
    m.redraw()
  }

  startPcap(vnode) {
    const ifsSelector = vnode.dom.querySelector('[name=ifs]')
    const opt = ifsSelector.options[ifsSelector.selectedIndex]
    const ifs = opt.value
    const ifsName = opt.getAttribute('data-name')
    Object.assign(Tab.options, {
      ifs,
      ifsName
    })
    Tab.page = 'pcap'

    setTimeout(() => {
      const factory = new SessionFactory()
      factory.options = Profile.current.object
      factory.networkInterface = Tab.options.ifs || ''
      factory.snaplen = Profile.current.get('_', 'snaplen')
      for (const layer of Session.linkLayers) {
        factory.registerLinkLayer(layer)
      }
      for (const diss of Session.dissectors) {
        factory.registerDissector(diss)
      }
      for (const diss of Session.streamDissectors) {
        factory.registerStreamDissector(diss)
      }
      factory.create().then((sess) => {
        if (Tab.options.ifs) {
          sess.startPcap()
        }
        Channel.emit('core:pcap:session-created', sess)
      }, (err) => {
        console.log(err)
      })
      GlobalChannel.emit('core:tab:set-name', Tab.id, `${Tab.options.ifsName} @ Live Capture`)
    }, 100)
  }

  oncreate(vnode) {
    if (Tab.options.files) {
      Tab.page = 'pcap'
      m.redraw()

      setTimeout(() => {
        const factory = new SessionFactory()
        factory.options = Profile.current.object
        factory.networkInterface = Tab.options.ifs || ''
        for (const layer of Session.linkLayers) {
          factory.registerLinkLayer(layer)
        }
        for (const diss of Session.dissectors) {
          factory.registerDissector(diss)
        }
        for (const diss of Session.streamDissectors) {
          factory.registerStreamDissector(diss)
        }
        File.loadFrames(Tab.options.files).then((results) => {
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

  view(vnode) {
    return <main>
      <section>
        <h1>Live capture</h1>
        {
          m(PermissionMassage, {})
        }
        <ul>
        <li>
          <select name="ifs">
          {
            Pcap.devices.map((dev) => {
              let name = dev.name
              if (name !== dev.id && process.platform !== 'win32') {
                name += ` - ${dev.id}`
              }
              return <option value={ dev.id } data-name={ name }>{ name }</option>
            })
          }
          </select>
        </li>
        <li>
          <input
            type="button"
            value="Start Live Capture"
            disabled={ !this.loaded }
            onclick={ ()=>{ this.startPcap(vnode) } }
          ></input>
        </li>
        </ul>
      </section>
    </main>
  }
}
