import { Tab } from 'deplug'
import { Pcap } from 'plugkit'
import m from 'mithril'

class PermissionMassage {
  view(vnode) {
    if (Pcap.permission) {
      return <p><i class="fa fa-check"></i> Live captures are ready.</p>
    }
    switch (process.platform) {
      case 'darwin':
        return <p>
        <i class="fa fa-exclamation-triangle"></i> Live captures are NOT ready.<br></br>
          Could not access /dev/bpf*.
          Please check if the Deplug Helper Tool has been installed correctly.
        </p>
      case 'linux':
        return <p>
        <i class="fa fa-exclamation-triangle"></i> Live captures are NOT ready.<br></br>
        The program does not have enough capabilities to start a live capture.<br></br>
        Please run setcap to the executable and don’t forget change RPATH.
        <ul>
          <li>$ DEPLAG_BIN={process.execPath}</li>
          <li>$ patchelf --set-rpath $(dirname $DEPLAG_BIN) $DEPLAG_BIN</li>
          <li>$ sudo setcap cap_net_raw,cap_net_admin=eip $DEPLAG_BIN</li>
        </ul>
        </p>
      case 'win32':
        return <p>
        <i class="fa fa-exclamation-triangle"></i> Live captures are NOT ready.<br></br>
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
  }

  oncreate(vnode) {
    if (Tab.options.files) {
      Tab.page = 'pcap'
      m.redraw()
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
          <input type="text" ref="filter" placeholder="filter (BPF)"></input>
        </li>
        <li>
          <label>
            <input type="checkbox" ref="promisc"></input>
            Promiscuous mode
          </label>
        </li>
        <li>
          <input
            type="button"
            value="Start Live Capture"
            onclick={ ()=>{ this.startPcap(vnode) } }
          ></input>
        </li>
        </ul>
      </section>
      <section>
        <h1>Import local file</h1>
        <input
          type="button"
          value="Open File..."
          onclick={ ()=>{ Tab.page = 'pcap' } }
        ></input>
      </section>
    </main>
  }
}
