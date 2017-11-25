import Dialog from '../../lib/dialog'
import PcapDialog from './pcap-dialog'
import m from 'mithril'

export default class PcapView {
  constructor () {
    this.itemHeight = 80
    this.stat = {
      frames: 0,
      queue: 0,
    }
  }

  view () {
    return [
      m('header', []),
      m('nav', [
        m('div', { style: { height: `${this.stat.frames * this.itemHeight}px` } })
      ]),
      m('main', [
        m('h1', ['Deplug'])
      ]),
      m('div', { class: 'notification' })
    ]
  }

  oncreate (vnode) {
    const resizeObserver = new ResizeObserver(() => {
      m.redraw()
    })
    resizeObserver.observe(vnode.dom)

    const dialog = new Dialog(PcapDialog)
    dialog.show({ cancelable: false }).then(async (ifs) => {
      const sess = await deplug.session.create(ifs)
      sess.startPcap()
      sess.on('frame', (stat) => {
        deplug.logger.debug(stat)
        this.stat = stat
        m.redraw()
      })
    })
  }
}
