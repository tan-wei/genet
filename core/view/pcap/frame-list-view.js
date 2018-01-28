import DefaultSummary from './default-summary'
import m from 'mithril'
import throttle from 'lodash.throttle'

class FrameView {
  view (vnode) {
    if (!this.frame) {
      const { sess, key } = vnode.attrs;
      [this.frame] = sess.getFrames(key, 1)
    }
    if (!this.frame) {
      return m('div')
    }
    const { id } = this.frame.primaryLayer
    const renderer = deplug.session.layerRenderer(id) || DefaultSummary
    return m('div', {
      class: 'frame',
      style: vnode.attrs.style,
      'data-layer': this.frame.primaryLayer.tags.join(' '),
      onclick: () => {
        deplug.action.emit('core:frame:selected', [this.frame])
      },
    }, [
      m('div', { class: 'header' }, [
        m('span', [deplug.session.token(id).name]),
        m('span', { class: 'right' }, [`${this.frame.length} bytes`])
      ]),
      m('div', { class: 'content' }, [
        m(renderer, { layer: this.frame.primaryLayer })
      ])
    ])
  }
}

export default class FrameListView {
  constructor () {
    this.itemHeight = 60
    this.height = 0
    this.scrollTop = 0
    this.prevFrames = 0
    this.mapHeight = 256
    this.updateMapThrottle = throttle((vnode) => {
      this.updateMap(vnode)
    }, 500)
  }

  updateMap (vnode) {
    const { sess } = vnode.attrs
    if (sess && sess.frame.frames > 0) {
      const dummy = this.dummyCanvas
      const ctx = dummy.getContext('2d')
      const frames = (sess.filter.main
        ? sess.filter.main.frames
        : sess.frame.frames)
      ctx.clearRect(0, 0, 1, this.mapHeight)
      if (frames > 0) {
        for (let line = 0; line < this.mapHeight; line += 1) {
          let index = Math.floor(frames / this.mapHeight * (line + 0.5))
          if (sess.filter.main) {
            index = sess.getFilteredFrames('main', index, 1)[0] - 1
          }
          const [frame] = sess.getFrames(index, 1)
          dummy.setAttribute('data-layer', frame.primaryLayer.tags.join(' '))
          ctx.fillStyle =
            getComputedStyle(dummy, null).getPropertyValue('background-color')
          ctx.fillRect(0, line, 1, 1)
        }
      }
      const data = dummy.toDataURL('image/png')
      this.barStyle.textContent = `
      nav.frame-list .padding, nav.frame-list::-webkit-scrollbar {
        background-image: url(${data});
      }
      `
    }
  }

  view (vnode) {
    const { frame, filter } = vnode.attrs.sess
    const frames = filter.main
      ? filter.main.frames
      : frame.frames
    const visibleItems = Math.min(
      Math.floor(this.height / this.itemHeight) + 2, frames)
    const startIndex = Math.floor(this.scrollTop / this.itemHeight)
    const listStyle = { height: `${frames * this.itemHeight}px` }

    const filteredFrames =
      vnode.attrs.sess.getFilteredFrames('main', startIndex, visibleItems)

    const items = []
    for (let index = 0; index < visibleItems; index += 1) {
      const seq = (filter.main)
        ? filteredFrames[index] - 1
        : index + startIndex
      const itemStyle = {
        height: `${this.itemHeight}px`,
        top: `${(index + startIndex) * this.itemHeight}px`,
      }
      items.push(m(FrameView, {
        style: itemStyle,
        key: seq,
        sess: vnode.attrs.sess,
      }))
    }
    return m('nav', { class: 'frame-list' }, [
      m('style', { class: 'scrollbar-style' }),
      m('canvas', {
        style: 'opacity: 0; position: absolute;',
        class: 'dummy-item',
        width: '1',
        height: this.mapHeight,
      }),
      m('div', {
        class: 'padding',
        style: { height: `${this.itemHeight * frames}px` },
      }),
      m('div', {
        class: 'container',
        style: listStyle,
      }, items)
    ])
  }

  onupdate (vnode) {
    const { frames } = vnode.attrs.sess.frame
    if (this.prevFrames !== frames) {
      this.prevFrames = frames
      if (!vnode.attrs.scrollLock) {
        vnode.dom.scrollTop = vnode.dom.scrollHeight - vnode.dom.clientHeight
      }
    }

    this.updateMapThrottle(vnode)
  }

  oncreate (vnode) {
    this.dummyCanvas = vnode.dom.parentNode.querySelector('.dummy-item')
    this.barStyle = vnode.dom.parentNode.querySelector('.scrollbar-style')

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
