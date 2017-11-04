import m from 'mithril'
import path from 'path'
import url from 'url'

export default class RootView {
  constructor () {
    this.tabs = [{
      id: 'preferences',
      src: 'preference.htm',
      argv: deplug.argv,
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
      m('main', this.tabs.map((tab) => {
        const localUrl = url.format({
          protocol: 'file',
          slashes: true,
          pathname: path.join(__dirname, tab.src),
          search: JSON.stringify(tab.argv),
        })
        return m('webview', {
          key: tab.id,
          src: localUrl,
          nodeintegration: true,
        })
      }))
    ]
  }
}
