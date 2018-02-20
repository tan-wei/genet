import { AttributeValueItem } from './value'
import DefaultSummary from './default-summary'
import m from 'mithril'
import parseColor from 'parse-color'
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
    return m('div', {
      class: 'frame',
      style: vnode.attrs.style,
      'data-layer': this.frame.primaryLayer.tags.join(' '),
      onmousedown: () => {
        deplug.action.emit('core:frame:selected', [this.frame])
      },
    }, [
      m('div', { class: 'header' },
        [
          m('input', { type: 'checkbox' })
        ].concat(
          vnode.attrs.columns.map((column) =>
            m('span', { class: 'column' }, [
              column.func(this.frame)
            ]))))
    ])
  }
}

export default class FrameListView {
  constructor () {
    this.itemHeight = 30
    this.height = 0
    this.scrollTop = 0
    this.prevFrames = 0
    this.mapHeight = 256
    this.columns = []
    this.updateMapThrottle = throttle((vnode) => {
      this.updateMap(vnode)
    }, 500)

    this.mapHeader = [
      0x42, 0x4d, 0x36, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00,
      0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0xff,
      0xff, 0xff, 0x01, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x13, 0x0b, 0x00, 0x00, 0x13, 0x0b, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    ]
    this.mapBuffer = Buffer.allocUnsafe(
      this.mapHeader.length + (4 * this.mapHeight))
    for (let index = 0; index < this.mapHeader.length; index += 1) {
      this.mapBuffer[index] = this.mapHeader[index]
    }
  }

  updateMap (vnode) {
    const { sess } = vnode.attrs
    if (sess && sess.frame.frames > 0 && this.dummyItem) {
      const frames = (sess.filter.main
        ? sess.filter.main.frames
        : sess.frame.frames)
      if (frames > 0) {
        for (let line = 0; line < this.mapHeight; line += 1) {
          let index = Math.floor(frames / this.mapHeight * (line + 0.5))
          if (sess.filter.main) {
            index = sess.getFilteredFrames('main', index, 1)[0] - 1
          }
          const [frame] = sess.getFrames(index, 1)
          this.dummyItem.setAttribute('data-layer',
            frame.primaryLayer.tags.join(' '))
          const [red, green, blue] =
            parseColor(getComputedStyle(this.dummyItem, null)
              .getPropertyValue('background-color')).rgb
          const offset = this.mapHeader.length
          this.mapBuffer[offset + (line * 4) + 0] = blue
          this.mapBuffer[offset + (line * 4) + 1] = green
          this.mapBuffer[offset + (line * 4) + 2] = red
        }
      }
      const data = `data:image/bmp;base64,${this.mapBuffer.toString('base64')}`
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
        columns: this.columns,
      }))
    }
    return m('nav', { class: 'frame-list' }, [
      m('style', { class: 'scrollbar-style' }),
      m('div', {
        style: 'display: none;',
        class: 'dummy-item',
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
    const { sess, viewState } = vnode.attrs
    const { frame, filter } = sess
    const frames = filter.main
      ? filter.main.frames
      : frame.frames
    if (this.prevFrames !== frames) {
      this.updateMapThrottle(vnode)
      this.prevFrames = frames
      if (!viewState.scrollLock) {
        vnode.dom.scrollTop = vnode.dom.scrollHeight - vnode.dom.clientHeight
      }
    }
  }

  oncreate (vnode) {
    this.dummyItem = vnode.dom.parentNode.querySelector('.dummy-item')
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

    this.columns = [
      {
        func: (frame) => {
          const { id } = frame.primaryLayer
          return m('span', [deplug.session.token(id).name])
        },
      }
    ]

    const compiler = deplug.session.createFilterCompiler()
    this.columns.push(...['_.src', '_.dst', '$.payload.length']
      .map((fitler) => {
        const filerFunc = compiler.compile(fitler, { rawResult: true }).built
        return {
          func: (frame) => {
            const result = filerFunc(frame)
            let renderer = AttributeValueItem
            if (typeof result === 'object' &&
              result.constructor.name === 'Attr') {
              renderer = deplug.session.attrRenderer(result.type) || renderer
              return m(renderer, { attr: result })
            }
            return m(renderer, { attr: { value: result } })
          },
        }
      }))

    this.columns.push({
      func: (frame) => {
        const { id } = frame.primaryLayer
        const renderer = deplug.session.layerRenderer(id) || DefaultSummary
        return m(renderer, { layer: frame.primaryLayer })
      },
    })
  }
}
