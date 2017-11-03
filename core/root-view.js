import m from 'mithril'

export default class RootView {
  constructor () {
    this.tabs = [{
      id: 'preferences',
      src: 'https://www.github.com/',
    }]
  }

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
      m('main', this.tabs.map((tab) =>
        m('webview', {
          key: tab.id,
          src: tab.src,
        })))
    ]
  }
}
