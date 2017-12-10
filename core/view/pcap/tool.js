import m from 'mithril'

class PanelView {
  view () {
    return m('div')
  }

  oncreate (vnode) {
    this.container = document.createElement('div')
    this.container.classList.add('panel')
    const node = this.container.attachShadow({ mode: 'open' })
    m.mount(node, vnode.attrs.component)

    const styleTag = document.createElement('style')
    styleTag.textContent = vnode.attrs.style
    node.appendChild(styleTag)

    vnode.dom.appendChild(this.container)
  }

  onbeforeremove () {
    this.container.remove()
  }
}

class PcapTabView {
  constructor () {
    this.activeTab = ''
  }

  view (vnode) {
    const { tabs } = vnode.attrs
    if (tabs.length > 0) {
      const panel = deplug.workspace.panel(this.activeTab)
      if (typeof panel === 'undefined') {
        [this.activeTab] = tabs
      }
    }

    const panels = tabs.map((tab) => deplug.workspace.panel(tab))
      .filter((panel) => typeof panel !== 'undefined')
    return ('div', [
      m('div', { class: 'tab-container' }, panels.map((panel) =>
        m('span', { active: panel.id === this.activeTab }, panel.name))),
      m('div', { class: 'panel-container' }, panels.map((panel) =>
        m(PanelView, Object.assign(panel,
          { active: panel.id === this.activeTab }))))
    ])
  }
}

export default class PcapToolView {
  view () {
    const panels = deplug.workspace.panelLayout.bottom || []
    return m('div', { class: 'tool-view' },
      panels.map((group) => m(PcapTabView, { tabs: group })))
  }
}
