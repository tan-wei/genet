import m from 'mithril'
import { Pcap, SessionFactory } from 'plugkit'

class ConfigView {
  constructor() {
    this.frame = {frames: 0}
    let factory = new SessionFactory()
    factory.networkInterface = Pcap.devices[0].id
    factory.create().then((s) => {
      s.on('frame', (stat) => {
        this.frame = stat
        m.redraw()
      })
      s.startPcap()
    }, (err) => {
      console.log(err)
    })
  }

  view(vnode) {
    return <div>
        <h1>Live capture</h1>
        <p>
          Permission: { Pcap.permission ? 'OK' : 'NG'}
        </p>
        <p>
          Frames: { `${this.frame.frames}` }
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
  view(vnode) {
    return <h1><a href="#!/">Pcap Pcap</a></h1>
  }
}

export default {
  '/': ConfigView,
  '/pcap': PcapView
}
