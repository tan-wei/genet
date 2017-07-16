import m from 'mithril'
import moment from 'moment'
import { Channel, Profile } from 'deplug'

export default class StatView {
  constructor() {
    this.stat = {frames: 0, queue: 0}
    Channel.on('core:pcap:session-created', (sess) => {
      sess.on('frame', (stat) => {
        this.stat = stat
        m.redraw()
      })
    })
  }

  view(vnode) {
    return <div class="stat-view">
      <ul>
        <li>
          <i class="fa fa-flask"></i>
          <label> Dissected Frames: </label>
          <span> { this.stat.frames } </span>
        </li>
      </ul>
    </div>
  }
}
