import m from 'mithril'

class PcapTabView {
  view (vnode) {
    const { tabs } = vnode.attrs
    return ('div', tabs.map((tab) => {
      const panel = deplug.workspace.panel(tab)
      if (typeof panel === 'undefined') {
        return m('p')
      }
      return m(deplug.workspace.panel(tab), {})
    }))
  }
}

export default class PcapToolView {
  view () {
    const panels = deplug.workspace.panelLayout.bottom || []
    return m('div', { class: 'tool-view' },
      panels.map((group) => m(PcapTabView, { tabs: group })))
  }
}
