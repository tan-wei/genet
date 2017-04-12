import m from 'mithril'
import { Channel, Panel } from 'deplug'
import { Pcap, SessionFactory } from 'plugkit'

class FrameItem {
  oninit(vnode) {
    this.frame = vnode.attrs.session.getFrames(vnode.attrs.seq - 1, 1)[0]
  }

  view(vnode) {
    let seq = vnode.attrs.seq
    let itemHeight = vnode.attrs.itemHeight
    const prot = (this.frame.length % 2) ? 'tcp' : 'udp'
    return <div
      class="item"
      data-layer={`eth ipv4 ${prot}`}
      style={{
        height: `${itemHeight}px`,
        top: `${(seq - 1) * itemHeight}px`
      }}
    >
    { `>>>> ${seq} ${this.frame.timestamp} ${this.frame.length}` }
    </div>
  }
}

export default class FrameView {
  constructor() {
    this.frame = {frames: 0}
    this.session = null
    this.viewScrollTop = 0
    this.viewHeight = 0
    Channel.on('core:pcap:session-created', (sess) => {
      this.session = sess
      this.session.on('frame', (stat) => {
        this.frame = stat
        m.redraw()
      })
      m.redraw()
    })

    let factory = new SessionFactory()
    factory.networkInterface = Pcap.devices[0].id
    factory.create().then((sess) => {
      sess.startPcap()
      Channel.emit('core:pcap:session-created', sess)
    }, (err) => {
      console.log(err)
    })
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

    if (this.session) {
      const dummy = vnode.dom.querySelector('.dummy-item')
      const ctx = dummy.getContext('2d')
      for (let i = 0; i < 100; ++i) {
        const index = Math.floor(this.session.frame.frames / 100 * i)
        const length = this.session.getFrames(index, 1)[0].length
        if (length % 2) {
          dummy.setAttribute('data-layer', "eth ipv4 tcp")
        } else {
          dummy.setAttribute('data-layer', "eth ipv4 udp")
        }
        ctx.fillStyle = getComputedStyle(dummy, null).getPropertyValue("background-color")
        ctx.fillRect(0, i, 1, 1)
      }
      const data = dummy.toDataURL("image/png")
      document.styleSheets[0]
        .addRule('.frame-view::-webkit-scrollbar', `background-image: url(${data});`)
      // TODO: remove rule
    }
  }

  view(vnode) {
    const itemHeight = 40
    const viewHeight = this.frame.frames * itemHeight
    const margin = 5
    const begin = Math.max(0,
      Math.floor(this.viewScrollTop / itemHeight) - margin)
    const end = Math.min(begin +
      Math.ceil(this.viewHeight / itemHeight) + margin * 2, this.frame.frames)

    return <div class="frame-view">
      <canvas
        style="opacity: 0; position: absolute;"
        class="dummy-item"
        data-layer="eth ipv4 tcp"
        width="1"
        height="100"
      ></canvas>
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
