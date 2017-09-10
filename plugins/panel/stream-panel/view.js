import m from 'mithril'
import moment from 'moment'
import { Channel } from 'deplug'

export default class StreamView {
  constructor() {
    this.session = null
    Channel.on('core:frame:selected', (frames) => {
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
        console.log(stat)
        m.redraw()
      })
    })
  }

  view(vnode) {
    return <div class="stream-view"></div>
  }
}
