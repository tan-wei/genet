import Style from '../../lib/style'
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

    const themeStyleTag = document.createElement('style')
    themeStyleTag.id = 'theme-style'
    node.appendChild(themeStyleTag)

    const globalStyleTag = document.createElement('style')
    globalStyleTag.id = 'global-style'
    node.appendChild(globalStyleTag)

    const customStyleTag = document.createElement('style')
    customStyleTag.id = 'custom-style'
    customStyleTag.textContent = vnode.attrs.style
    node.appendChild(customStyleTag)

    const loader = new Style()
    loader.applyCommon(node)

    vnode.dom.appendChild(this.container)
  }

  onbeforeremove () {
    this.container.remove()
  }
}

class PcapTabView {
  oncreate () {
    this.activeTab = deplug.workspace.get('_.pcap.activeTab', '')
  }

  view (vnode) {
    const { tabs } = vnode.attrs

    const tabOrders = deplug.workspace.get('_.pcap.tabOrders', [])
    tabs.sort((lhs, rhs) => {
      const lhsIndex = tabOrders.indexOf(lhs)
      const rhsIndex = tabOrders.indexOf(rhs)
      if (lhsIndex < 0 && rhsIndex < 0) {
        return lhs > rhs
          ? 1
          : -1
      }
      if (lhsIndex >= 0 && rhsIndex < 0) {
        return -1
      }
      if (lhsIndex < 0 && rhsIndex >= 0) {
        return 1
      }
      return lhsIndex - rhsIndex
    })

    let activeTabIndex = tabs.findIndex((tab) => tab === this.activeTab)
    if (activeTabIndex < 0) {
      activeTabIndex = tabs.findIndex((tab) =>
        typeof deplug.workspace.panel(tab) !== 'undefined')
    }
    if (activeTabIndex >= 0) {
      this.activeTab = tabs[activeTabIndex]
    }

    const panels = tabs.map((tab) => deplug.workspace.panel(tab))
      .filter((panel) => typeof panel !== 'undefined')
    return ('div', [
      m('div', { class: 'tab-container' }, panels.map((panel) =>
        m('span', {
          active: panel.id === this.activeTab,
          onclick: () => {
            this.activeTab = panel.id
            deplug.workspace.set('_.pcap.activeTab', this.activeTab)
          },
        }, panel.name))),
      m('div', { class: 'panel-container' }, panels.map((panel) =>
        m('div', {
          class: 'panel-wrapper',
          active: panel.id === this.activeTab,
        }, [
          m(PanelView, Object.assign(panel,
            { active: panel.id === this.activeTab }))
        ])))
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
