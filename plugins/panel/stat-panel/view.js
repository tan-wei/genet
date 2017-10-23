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
    return m('div', {class:'stat-view'}, [
      m('ul', [
        m('li', [
          m('i', {class:'fa fa-flask'}),
          m('label', [' Dissected Frames: ']),
          m('span', [' ',this.stat.frames,' '])
        ])
      ])
    ])
  }
}
