import TabView from './tab-view'
import m from 'mithril'

export default class InputDialog {
  view (vnode) {
    const { callback } = vnode.attrs
    const panels = genet.workspace.panelLayout['dialog:input'] || []
    return m('div', { class: 'tool-view' },
      panels.map((group) => m(TabView, {
        tabs: group,
        attrs: { callback },
      })))
  }
}
