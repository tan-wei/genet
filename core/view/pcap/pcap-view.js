import Dialog from '../../lib/dialog'
import PcapDialog from './pcap-dialog'
import m from 'mithril'

class FrameView {
  view (vnode) {
    if (!this.frame) {
      const { sess, key } = vnode.attrs;
      [this.frame] = sess.getFrames(key, 1)
    }
    if (!this.frame) {
      return m('div')
    }
    return m('div', {
      class: 'frame',
      style: vnode.attrs.style,
    }, [this.frame.length])
  }
}

class FrameListView {
  constructor () {
    this.itemHeight = 60
    this.height = 0
    this.scrollTop = 0
  }

  view (vnode) {
    const visibleItems = Math.min(
      Math.floor(this.height / this.itemHeight) + 2, vnode.attrs.stat.frames)
    const startIndex = Math.floor(this.scrollTop / 60)
    const listStyle =
      { height: `${vnode.attrs.stat.frames * this.itemHeight}px` }
    const items = []
    for (let index = 0; index < visibleItems; index += 1) {
      const seq = index + startIndex
      const itemStyle = {
        height: `${this.itemHeight}px`,
        top: `${seq * this.itemHeight}px`,
      }
      items.push(m(FrameView, {
        style: itemStyle,
        key: seq,
        sess: vnode.attrs.sess,
      }))
    }
    return m('nav', [
      m('div', { style: listStyle }, items)
    ])
  }

  oncreate (vnode) {
    const resizeObserver = new ResizeObserver((entries) => {
      for (const entry of entries) {
        if (entry.target === vnode.dom) {
          this.height = entry.contentRect.height
          m.redraw()
        }
       }
    })
    resizeObserver.observe(vnode.dom)
    vnode.dom.addEventListener('scroll', (event) => {
      this.scrollTop = event.target.scrollTop
      m.redraw()
    })
  }
}

export default class PcapView {
  constructor () {
    this.stat = {
      frames: 0,
      queue: 0,
    }
    this.sess = null
  }

  view () {
    return [
      m('header', []),
      m(FrameListView, {
        stat: this.stat,
        sess: this.sess,
      }),
      m('main', [
        m('h1', ['Deplug'])
      ]),
      m('div', { class: 'notification' })
    ]
  }

  oncreate () {
    const dialog = new Dialog(PcapDialog)
    dialog.show({ cancelable: false }).then(async (ifs) => {
      const sess = await deplug.session.create(ifs)
      this.sess = sess
      sess.startPcap()
      sess.on('frame', (stat) => {
        deplug.logger.debug(stat)
        this.stat = stat
        m.redraw()
      })
    })
  }
}
