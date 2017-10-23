import {
  Config
} from 'deplug'
import m from 'mithril'
import objpath from 'object-path'
export default class AboutView {
  constructor() {
    this.version = objpath.get(Config.deplug, 'version', 'n/a')
    this.electronVersion = objpath.get(Config.deplug, 'devDependencies.negatron', 'n/a')
  }
  view(vnode) {
    return [
      m('h1', ['About Deplug']),
      m('table', [
        m('tr', [m('td', ['Deplug version']), m('td', [this.version])]),
        m('tr', [m('td', ['Negatron version']), m('td', [this.electronVersion])])
      ])
    ]
  }
}
