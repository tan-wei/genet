import { Channel } from 'deplug'
import m from 'mithril'

export default class View {
  constructor() {
    this.capture = false
    Channel.on('core:pcap:session-created', (sess) => {
      this.session = sess
      this.session.on('status', (stat) => {
        this.capture = stat.capture
        m.redraw()
      })
    })
  }

  oncreate(vnode) {

  }

  view(vnode) {
    if (this.capture) {
      return <a href="javascript:void(0)"
        onclick={ () => { this.session.stopPcap() } }
      >
      <i class="fa fa-pause-circle"></i> Pause</a>
    } else {
      return <a href="javascript:void(0)"
        onclick={ () => { this.session.startPcap() } }
      >
      <i class="fa fa-play-circle"></i> Start</a>
    }
  }
}
