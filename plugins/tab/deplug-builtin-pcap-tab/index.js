import m from 'mithril'
import { Panel } from 'deplug'
import { Pcap } from 'plugkit'

class ConfigView {
  constructor() {

  }

  view(vnode) {
    return <div>
        <h1>Live capture</h1>
        <p>
          Permission: { Pcap.permission ? 'OK' : 'NG'}
        </p>
        <ul>
          {
            Pcap.devices.map((dev) => {
              return <li>{ dev.name }</li>
            })
          }
        </ul>
        <h1>Import local file</h1>
        <input
          type="button"
          value="Choose pcap file..."
          onclick={ ()=>{ location.hash = "!/pcap" } }
        ></input>
      </div>
  }
}

class PcapView {
  constructor() {

  }

  oncreate(vnode) {
    Panel.registerSlot('pcap-top', (comp) => {
      m.mount(vnode.dom, comp)
    })
  }

  view(vnode) {
    return <div id="pcap-top"></div>
  }
}

export default {
  '/pcap': ConfigView,
  '/': PcapView
}
