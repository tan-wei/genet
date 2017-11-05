import Dialog from '../../lib/dialog'
import PcapDialog from './pcap-dialog'
import m from 'mithril'

export default class PcapView {
  view () {
    return [
      m('header', []),
      m('nav', [
        m('div', { style: { height: '1000px' } })
      ]),
      m('main', [
        m('h1', ['Deplug'])
      ])
    ]
  }

  oncreate () {
    const dialog = new Dialog(PcapDialog)
    dialog.show({ cancelable: false }).then((result) => console.log(result))
  }
}
