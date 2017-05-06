import m from 'mithril'
import moment from 'moment'
import { Channel, Profile } from 'deplug'

class BinaryItem {
  view(vnode) {
    const { layout } = Profile.current['$deplug-builtin-binary-panel']
    const hex = layout.includes('hex')
    const ascii = layout.includes('ascii')
    const payload = vnode.attrs.payload
    return <div class="binary-view">
      <ul class="hex-list" style={{display: hex ? 'block' : 'none'}}>
        {
          (new Array(Math.ceil(payload.length / 16))).fill().map((_, line) => {
            const slice = payload.slice(line * 16, (line + 1) * 16)
            return <li>
              {
                (new Array(slice.length)).fill().map((_, byte) => {
                  const index = line * 16 + byte
                  return <span>{ ('0' + payload[index].toString(16)).slice(-2) }</span>
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
                  const char = payload[line * 16 + byte]
                  const ascii = (0x21 <= char && char <= 0x7e) ? String.fromCharCode(char) : '.'
                  return <span>{ ascii }</span>
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
    Channel.on('core:frame:selected', (frames) => {
      this.frames = frames
      m.redraw()
    })
  }

  view(vnode) {
    if (!this.frames.length) {
      return <div class="binary-view">No frames selected</div>
    }
    return this.frames.map((frame) => {
      return m(BinaryItem, {payload: frame.rootLayer.payload})
    })
  }
}
