import TabView from './tab-view'
import m from 'mithril'

export default class InputDialog {
  view () {
    const panels = deplug.workspace.panelLayout['dialog:input'] || []
    return m('div', { class: 'tool-view' },
      panels.map((group) => m(TabView, { tabs: group })))
  }
}
