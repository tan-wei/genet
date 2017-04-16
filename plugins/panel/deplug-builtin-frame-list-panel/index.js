import throttle from 'lodash.throttle'
import m from 'mithril'
import { Channel, Panel } from 'deplug'
import { Pcap, SessionFactory } from 'plugkit'

class FrameItem {
  constructor() {
    this.attrs = {
      'layer-confidence': 'primaryLayer.confidence',
      'frame-length': 'length',
      'frame-capture-length': 'rootLayer.payload.length',
    }
    this.columns = [
      {name: 'No', value: 'seq'},
      {name: 'Protocol', value: 'primaryLayer.name'},
      {name: 'Length', value: 'length'},
      {name: 'Summary', value: 'primaryLayer.summary'},
    ]
  }

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
      data-layer-confidence={1}
      data-frame-length={this.frame.length}
      data-frame-capture-length={this.frame.rootLayer.payload.length}
      style={{
        height: `${itemHeight}px`,
        top: `${(seq - 1) * itemHeight}px`
      }}
    >
    { `${seq} ${this.frame.primaryLayer.name} ${this.frame.length}` }
    </div>
  }
}

export default class FrameView {
  constructor() {
    this.frame = {frames: 0}
    this.session = null
    this.viewScrollTop = 0
    this.viewHeight = 0

    this.updateMapThrottle = throttle((vnode) => {
      this.updateMap(vnode)
    }, 200)

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
    this.updateMapThrottle(vnode)
  }

  updateMap(vnode) {
    if (this.session && this.session.frame.frames > 0) {
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

      const css = document.styleSheets[0]
      const rules = css.cssRules
      for (let i = rules.length - 1; i >= 0; --i) {
        if (rules[i].cssText
            .startsWith('.frame-view::-webkit-scrollbar { background-image:')) {
          css.deleteRule(i)
          break
        }
      }
      css.addRule('.frame-view::-webkit-scrollbar',
        `background-image: url(${data});`)
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
