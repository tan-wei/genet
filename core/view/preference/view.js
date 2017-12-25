import General from './general'
import Version from './version'
import m from 'mithril'

export default class PrefernceView {
  constructor () {
    this.tabs = [{
      name: 'Pcap',
      component: General,
      attrs: {
        name: 'Pcap',
        prefix: '_.pcap.',
      },
    }, {
      name: 'Dissector',
      component: General,
      attrs: {
        name: 'Dissector',
        prefix: '_.dissector.',
      },
    }, {
      name: 'Package',
      component: General,
      attrs: {
        name: 'Package',
        prefix: '_.package.',
      },
    }, {
      name: 'Filter',
      component: General,
      attrs: {
        name: 'Filter',
        prefix: '_.filter.',
      },
    }, {
      name: 'Logger',
      component: General,
      attrs: {
        name: 'Logger',
        prefix: '_.logger.',
      },
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
          [m(item.component, item.attrs || {})]))),
      m('div', { class: 'notification' })
    ]
  }
}
