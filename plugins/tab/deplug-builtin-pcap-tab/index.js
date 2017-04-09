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

class FrameItem {
  oninit(vnode) {
    this.frame = vnode.attrs.session.getFrames(vnode.attrs.seq, 1)[0]
  }

  view(vnode) {
    let seq = vnode.attrs.seq
    let itemHeight = vnode.attrs.itemHeight
    return <div
      class="item"
      style={{
        height: `${itemHeight}px`,
        top: `${(seq - 1) * itemHeight}px`
      }}
    >
    { `>>>> ${seq} ${this.frame ? this.frame.timestamp : ''}` }
    </div>
  }
}

class FrameView {
  constructor() {
    this.frame = {frames: 0}
    this.session = null
    this.viewScrollTop = 0
    this.viewHeight = 0
  }

  oncreate(vnode) {
    this.onupdate(vnode)
    vnode.dom.addEventListener('scroll', (event) => {
      this.viewHeight = event.target.offsetHeight
      this.viewScrollTop = event.target.scrollTop
      m.redraw()
    })
  }

  onupdate(vnode) {
    this.viewHeight = vnode.dom.offsetHeight
    this.viewScrollTop = vnode.dom.scrollTop
  }

  view(vnode) {
    if (this.session === null && vnode.attrs.session !== null) {
      this.session = vnode.attrs.session
      this.session.on('frame', (stat) => {
        this.frame = stat
        m.redraw()
      })
    }
    const itemHeight = 40
    const viewHeight = this.frame.frames * itemHeight
    const begin = Math.floor(this.viewScrollTop / itemHeight)
    const end = Math.min(begin + Math.ceil(this.viewHeight / itemHeight) + 1, this.frame.frames)
    return <div class="frame-view">
      <div
        style={{height: `${viewHeight}px`}}
      >
        {
          (new Array(end - begin)).fill().map((dev, index) => {
            const id = index + begin + 1
            return m(FrameItem, {
              key: id,
              seq: id,
              itemHeight: itemHeight,
              session: this.session
            })
          })
        }
      </div>
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
