import m from 'mithril'

export default class Menu {
  view (vnode) {
    const { parent } = vnode.attrs
    return m('nav', parent.tabs.map((tab) => m('ul', [
      m('li', [
        m('a', {
          onclick: () => {
            parent.activeTab = tab.id
          },
          active: parent.activeTab === tab.id,
        }, [
          m('i', { class: `fa ${tab.icon}` }),
          ' ',
          tab.name,
          m('i', {
            class: 'fa fa-close close-button',
            style: {
              display: tab.system
                ? 'none'
                : 'inline-block',
            },
            onclick: () => {
              parent.removeTab(tab.id)
              m.redraw()
            },
          })
        ])
      ])
    ])))
  }

  onupdate () {
    const view = document.querySelector('webview[active]')
    if (view !== null) {
      view.focus()
    }
  }
}
