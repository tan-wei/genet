import m from 'mithril'
import moment from 'moment'
import { Channel, Profile } from 'deplug'

class BinaryItem {
  view(vnode) {
    const layout = Profile.current.get('binary-panel', 'layout')
    const hex = layout.includes('hex')
    const ascii = layout.includes('ascii')
    const payload = vnode.attrs.payload
    const range = vnode.attrs.range.length ? vnode.attrs.range : [-1, -1]
    return <div class="binary-view">
      <ul class="hex-list" style={{display: hex ? 'block' : 'none'}}>
        {
          (new Array(Math.ceil(payload.length / 16))).fill().map((_, line) => {
            const slice = payload.slice(line * 16, (line + 1) * 16)
            return <li>
              {
                (new Array(slice.length)).fill().map((_, byte) => {
                  const index = line * 16 + byte
                  return <span
                    data-selected={ range[0] <= index && index < range[1] }
                  >{ ('0' + payload[index].toString(16)).slice(-2) }</span>
                })
              }
            </li>
          })
        }
      </ul>
      <ul class="ascii-list" style={{display: ascii ? 'block' : 'none'}}>
        {
          (new Array(Math.ceil(payload.length / 16))).fill().map((_, line) => {
            const slice = payload.slice(line * 16, (line + 1) * 16)
            return <li>
              {
                (new Array(slice.length)).fill().map((_, byte) => {
                  const index = line * 16 + byte
                  const char = payload[index]
                  const ascii = (0x21 <= char && char <= 0x7e) ? String.fromCharCode(char) : '.'
                  return <span
                    data-selected={ range[0] <= index && index < range[1] }
                  >{ ascii }</span>
                })
              }
            </li>
          })
        }
      </ul>
    </div>
  }
}

export default class BinaryView {
  constructor() {
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

  view(vnode) {
    if (!this.frames.length) {
      return <div class="binary-view">No frames selected</div>
    }
    return this.frames.map((frame) => {
      return m(BinaryItem, {payload: frame.rootLayer.payloads[0].slices[0], range: this.range})
    })
  }
}
