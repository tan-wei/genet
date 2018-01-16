import General from './general'
import KeyBind from './keybind'
import Version from './version'
import m from 'mithril'

export default class PrefernceView {
  constructor () {
    this.tabs = [{
      name: 'Pcap',
      component: General,
      attrs: { prefix: '_.pcap.' },
    }, {
      name: 'Dissector',
      component: General,
      attrs: { prefix: '_.dissector.' },
    }, {
      name: 'Package',
      component: General,
      attrs: { prefix: '_.package.' },
    }, {
      name: 'Filter',
      component: General,
      attrs: { prefix: '_.filter.' },
    }, {
      name: 'KeyBind',
      component: KeyBind,
    }, {
      name: 'Debug',
      component: General,
      attrs: { prefix: '_.debug.' },
    }, {
      name: 'Version',
      component: Version,
    }]
    this.activeTab = 'Pcap'
  }

  view () {
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
