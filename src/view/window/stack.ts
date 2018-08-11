import WebView from '../../lib/webview'
import m from 'mithril'

export default class Stack {
  view(vnode) {
    const { parent } = vnode.attrs
    return [
      m('main', { class: 'loading' }, [
        m('div', { class: 'spinner' }, [
          m('div', { class: 'rect1' }),
          m('div', { class: 'rect2' }),
          m('div', { class: 'rect3' }),
          m('div', { class: 'rect4' }),
          m('div', { class: 'rect5' })
        ]),
      ])
    ].concat(
      m('main', parent.tabs.map((tab) => m(WebView, {
        tab,
        key: tab.id,
        active: parent.activeTab === tab.id,
      }))))
  }
}
