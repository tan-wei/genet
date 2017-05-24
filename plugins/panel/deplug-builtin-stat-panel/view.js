import m from 'mithril'
import moment from 'moment'
import { Channel, Profile } from 'deplug'

export default class StatView {
  constructor() {
    this.stat = {}
    Channel.on('core:pcap:session-created', (sess) => {
      sess.on('frame', (stat) => {
        this.stat = stat
        m.redraw()
      })
    })
  }

  view(vnode) {
    return <div class="stat-view">{ JSON.stringify(this.stat) }</div>
  }
}
