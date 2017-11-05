import Dialog from '../lib/dialog'
import m from 'mithril'

class PcapDialog {
  view (vnode) {
    return m('h1', {
      onclick: () => {
        vnode.attrs.callback('hello')
      },
    }, ['hello'])
  }
}

export default class PcapView {
  view () {
    return [
      m('nav', []),
      m('main', [
        m('h1', ['Deplug'])
      ])
    ]
  }

  oncreate () {
    const dialog = new Dialog(PcapDialog)
    dialog.show().then((result) => console.log(result))
  }
}
