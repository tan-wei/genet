import m from 'mithril'

export default class RootView {
  view () {
    return [
      m('nav', [
        m('ul', [
          m('li', ['Live Capture 1']),
          m('li', ['Live Capture 2'])
        ]),
        m('ul', [
          m('li', [m('i', { class: 'fa fa-cog' }), ' ', 'Preferences'])
        ])
      ]),
      m('main', ['hello'])
    ]
  }
}
