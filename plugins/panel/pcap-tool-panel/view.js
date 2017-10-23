import {
  Channel,
  Tab
} from 'deplug'
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
  view(vnode) {
    if (!Tab.options.ifs) {
      return m('a', {
        style: 'display: none;'
      })
    }
    if (this.capture) {
      return m('a', {
        href: 'javascript:void(0)',
        onclick: () => {
          this.session.stopPcap()
        }
      }, [
        m('i', {
          class: 'fa fa-pause-circle'
        }), ' Pause'
      ])
    } else {
      return m('a', {
        href: 'javascript:void(0)',
        onclick: () => {
          this.session.startPcap()
        }
      }, [
        m('i', {
          class: 'fa fa-play-circle'
        }), ' Start'
      ])
    }
  }
}
