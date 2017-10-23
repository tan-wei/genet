import {
  Plugin
} from 'deplug'
import PluginCard from './plugin-card'
import m from 'mithril'
export default class PluginView {
  constructor() {
    this.plugins = []
    Plugin.listPlugins().then((list) => {
      this.plugins = list
      m.redraw()
    })
  }
  view(vnode) {
    const userPlugins = this.plugins.filter((plugin) => !plugin.builtin)
    return [
      m('div', [
        m('h1', ['Plugin Settings']),
        m('h2', {
          style: {
            display: userPlugins.length ? 'block' : 'none'
          }
        }, ['User Plugins']),
        m('div', [
          userPlugins.map((plugin) => {
            return m(PluginCard, {
              pkg: plugin.pkg,
              installed: true,
              options: plugin.options
            })
          })
        ]),
        m('h2', ['Built-in Plugins']),
        m('div', [
          this.plugins.filter((plugin) => plugin.builtin).map((plugin) => {
            return m(PluginCard, {
              pkg: plugin.pkg,
              installed: true,
              options: plugin.options
            })
          })
        ])
      ])
    ]
  }
}
