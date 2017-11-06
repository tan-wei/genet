import General from '../preference/general'
import Install from '../preference/install'
import Plugin from '../preference/plugin'
import Version from '../preference/version'
import m from 'mithril'

export default class PrefernceView {
  constructor () {
    this.tabs = [{
      name: 'General',
      component: General,
    }, {
      name: 'Plugin',
      component: Plugin,
    }, {
      name: 'Install',
      component: Install,
    }, {
      name: 'Version',
      component: Version,
    }]
    this.activeTab = 'General'
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
          [m(item.component, {})])))
    ]
  }
}
