import m from 'mithril'
import moment from 'moment'
import { Channel } from 'deplug'

export default class StreamView {
  constructor() {
    this.session = null
    this.payloads = []
    this.frames = 0

    Channel.on('core:frame:selected', (frames) => {
      this.payloads = []
      this.frames = 0
      if (frames.length) {
        const id = frames[0].attr('tcp.streamId')
        if (id) {
          if (this.session) {
            this.session.setDisplayFilter('stream-panel', `tcp.streamId === ${id.value}`)
          }
        }
      }
      m.redraw()
    })

    Channel.on('core:pcap:session-created', (sess) => {
      this.session = sess
      this.session.on('filter', (stat) => {
        if ('stream-panel' in stat) {
          const { frames } = stat['stream-panel']
          const indices = this.session.getFilteredFrames('stream-panel', this.frames, frames - this.frames)
          for (const index of indices) {
            const tcp = this.session.getFrames(index - 1, 1)[0].layer('tcp')
            for (const payload of tcp.payloads) {
              if (payload.type === '@reassembled') {
                this.payloads.push(payload)
              }
            }
          }
          this.frames = frames
        }
        m.redraw()
      })
    })
  }

  view(vnode) {
    return <div class="stream-view">
    {
      this.payloads.map((payload) => {
        return <ul class="hex-list">
          {
            (new Array(Math.ceil(payload.slices[0].length / 16))).fill().map((_, line) => {
              const slice = payload.slices[0].slice(line * 16, (line + 1) * 16)
              return <li>
                {
                  (new Array(slice.length)).fill().map((_, byte) => {
                    const index = line * 16 + byte
                    return <span
                    >{ ('0' + payload.slices[0][index].toString(16)).slice(-2) }</span>
                  })
                }
              </li>
            })
          }
        </ul>
      })
    }
    </div>
  }
}
