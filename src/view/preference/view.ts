import General from './general'
import KeyBind from './keybind'
import Version from './version'
import m from 'mithril'

export default class PrefernceView {
  private activeTab: string
  private tabs: any[]

  constructor() {
    this.tabs = [{
      name: 'Package',
      component: General,
      attrs: { prefix: '_.package.' },
    }, {
      name: 'Dissector',
      component: General,
      attrs: { prefix: '_.dissector.' },
    }, {
      name: 'KeyBind',
      component: KeyBind,
    }, {
      name: 'Development',
      component: General,
      attrs: { prefix: '_.dev.' },
    }, {
      name: 'Version',
      component: Version,
    }]
    this.activeTab = 'Package'
  }

  view() {
    return [
      m('nav', [
        m('ul', this.tabs.map((item) => m('li', [
          m('a', {
            onclick: () => {
              this.activeTab = item.name
            },
            active: this.activeTab === item.name,
          }, [item.name])
        ])))
      ]),
      m('main', this.tabs.map((item) =>
        m('article', { active: this.activeTab === item.name },
          [m(item.component,
            Object.assign({ active: this.activeTab === item.name },
              item.attrs))]))),
      m('div', { class: 'notification' })
    ]
  }
}
