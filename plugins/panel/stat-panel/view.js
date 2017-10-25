import { Channel } from 'deplug'
import m from 'mithril'

export default class StatView {
  constructor () {
    this.stat = {
      frames: 0,
      queue: 0,
    }
    Channel.on('core:pcap:session-created', (sess) => {
      sess.on('frame', (stat) => {
        this.stat = stat
        m.redraw()
      })
    })
  }

  view () {
    return m('div', { class: 'stat-view' }, [
      m('ul', [
        m('li', [
          m('i', { class: 'fa fa-flask' }),
          m('label', [' Dissected Frames: ']),
          m('span', [' ', this.stat.frames, ' '])
        ])
      ])
    ])
  }
}
