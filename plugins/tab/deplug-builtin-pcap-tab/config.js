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
        <pre>
          $ setcap cap_net_raw,cap_net_admin=eip /usr/share/deplug/deplug
        </pre>
        <pre>
          $ patchelf --set-rpath $ORIGIN:$ORIGIN/lib/:/usr/share/deplug /usr/share/deplug/deplug
        </pre>
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

  view(vnode) {
    return <main>
        <h1>Live capture</h1>
        {
          m(PermissionMassage, {})
        }
        <select>
        {
          Pcap.devices.map((dev) => {
            return <option
              value={ dev.id }
              >{ (dev.id === dev.name) ? dev.id : `${dev.name} - ${dev.id}` }
            </option>
          })
        }
        </select>
        <h1>Import local file</h1>
        <input
          type="button"
          value="Choose pcap file..."
          onclick={ ()=>{ Tab.page = 'pcap' } }
        ></input>
      </main>
  }
}
