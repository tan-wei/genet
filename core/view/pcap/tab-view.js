import PanelView from './panel-view'
import m from 'mithril'

export default class TabView {
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
