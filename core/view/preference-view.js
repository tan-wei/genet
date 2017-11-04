import General from '../preference/general'
import Plugin from '../preference/plugin'
import m from 'mithril'

export default class PrefernceView {
  constructor () {
    this.tabs = [{
      name: 'General',
      component: General,
    }, {
      name: 'Plugin',
      component: Plugin,
    }]
    this.activeTab = 'General'
  }

  view () {
    return [
      m('nav', [
        m('ul', this.tabs.map((item) => m('li', [item.name])))
      ]),
      m('main', this.tabs.map((item) =>
        m('article', { active: this.activeTab === item.name },
          [m(item.component, {})])))
    ]
  }
}
