import m from 'mithril'

class PanelView {
  view () {
    return m('div')
  }

  oncreate (vnode) {
    this.container = document.createElement('div')
    this.container.classList.add('slot-wrapper')
    const node = this.container.attachShadow({ mode: 'open' })
    m.mount(node, vnode.attrs.component)
    vnode.dom.parentNode.appendChild(this.container)
  }

  onbeforeremove () {
    this.container.remove()
  }
}

class PcapTabView {
  view (vnode) {
    const { tabs } = vnode.attrs
    return ('div', tabs.map((tab) => {
      const panel = deplug.workspace.panel(tab)
      if (typeof panel === 'undefined') {
        return m('p')
      }
      return m(PanelView, { component: deplug.workspace.panel(tab) })
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
