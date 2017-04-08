import m from 'mithril'
import { Pcap, SessionFactory } from 'plugkit'

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

class FrameView {
  constructor() {
    this.frame = {frames: 0}
    this.session = null
  }

  view(vnode) {
    if (this.session === null && vnode.attrs.session !== null) {
      this.session = vnode.attrs.session
      this.session.on('frame', (stat) => {
        this.frame = stat
        m.redraw()
      })
    }
    return <div>
      {
        (new Array(this.frame.frames)).fill().map((dev) => {
          return <li>{ "dev.name" }</li>
        })
      }
    </div>
  }
}

class PcapView {
  constructor() {
    this.session = null
    let factory = new SessionFactory()
    factory.networkInterface = Pcap.devices[0].id
    factory.create().then((s) => {
      this.session = s
      s.startPcap()
      m.redraw()
    }, (err) => {
      console.log(err)
    })
  }

  view(vnode) {
    return m(FrameView, {session: this.session})
  }
}

export default {
  '/pcap': ConfigView,
  '/': PcapView
}
