import General from './general'
import KeyBind from './keybind'
import Version from './version'
import License from './license'
import genet from '@genet/api'
import m from 'mithril'

export default class PrefernceView {
  private activeTab: string
  private tabs: any[]

  constructor() {
    this.tabs = [{
      name: 'Decoder',
      component: General,
      attrs: { prefix: '_.decoder.' },
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
    }, {
      name: 'License',
      component: License,
    }]
    this.activeTab = 'Decoder'
  }

  oncreate() {
    genet.action.global.on('core:tab:reload', () => {
      window.location.reload()
    })
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
            { active: this.activeTab === item.name, ...item.attrs },
          )]))),
      m('div', { class: 'notification' })
    ]
  }
}
