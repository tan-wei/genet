import { Channel, Profile } from 'deplug'
import m from 'mithril'

class BinaryItem {
  view (vnode) {
    const layout = Profile.current.get('binary-panel.layout')
    const showHex = layout.includes('hex')
    const showAscii = layout.includes('ascii')
    const { payload } = vnode.attrs
    const range = vnode.attrs.range.length
    ? vnode.attrs.range
    : [-1, -1]
    return m('div', { class: 'binary-view' }, [
      m('ul', {
        class: 'hex-list',
        style: {
          display: showHex
          ? 'block'
          : 'none',
        },
      }, [
        (new Array(Math.ceil(payload.length / 16))).fill()
        .map((idx, line) => {
          const slice = payload.slice(line * 16, (line + 1) * 16)
          return m('li', [
            (new Array(slice.length)).fill()
            .map((item, byte) => {
              const index = (line * 16) + byte
              return m('span',
                { 'data-selected': range[0] <= index && index < range[1] },
                [(`0${payload[index].toString(16)}`).slice(-2)])
            })
          ])
        })
      ]),
      m('ul', {
        class: 'ascii-list',
        style: {
          display: showAscii
          ? 'block'
          : 'none',
        },
      }, [
        (new Array(Math.ceil(payload.length / 16))).fill()
        .map((str, line) => {
          const slice = payload.slice(line * 16, (line + 1) * 16)
          return m('li', [
            (new Array(slice.length)).fill()
            .map((item, byte) => {
              const index = (line * 16) + byte
              const char = payload[index]
              const ascii = (char >= 0x21 && char <= 0x7e)
                ? String.fromCharCode(char)
                : '.'
              return m('span',
                { 'data-selected': range[0] <= index && index < range[1] },
                [ascii])
            })
          ])
        })
      ])
    ])
  }
}
export default class BinaryView {
  constructor () {
    this.frames = []
    this.range = []
    Channel.on('core:frame:selected', (frames) => {
      this.frames = frames
      m.redraw()
    })
    Channel.on('core:frame:range-selected', (range) => {
      this.range = range
      m.redraw()
    })
  }
  view () {
    if (!this.frames.length) {
      return m('div', { class: 'binary-view' }, ['No frames selected'])
    }
    return this.frames.map((frame) => m(BinaryItem, {
      payload: frame.rootLayer.payloads[0].slices[0],
      range: this.range,
    }))
  }
}
