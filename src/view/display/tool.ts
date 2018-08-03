import TabView from './tab-view'
import genet from '@genet/api'
import m from 'mithril'

export default class PcapToolView {
  view() {
    const panels = genet.workspace.panelLayout['tools:bottom'] || []
    return m('div', { class: 'tool-view' },
      panels.map((group) => m(TabView, { tabs: group })))
  }
}
