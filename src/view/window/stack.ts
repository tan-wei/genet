import WebView from '../../lib/webview'
import m from 'mithril'

export default class Stack {
  view(vnode) {
    const { parent } = vnode.attrs
    return [
      m('main', { class: 'loading' }, [
        m('i', { class: 'fa fa-cog fa-spin fa-3x fa-fw' }),
        m('h2', ['Loading...'])
      ])
    ].concat(
      m('main', parent.tabs.map((tab) => m(WebView, {
        tab,
        key: tab.id,
        active: parent.activeTab === tab.id,
      }))))
  }
}
