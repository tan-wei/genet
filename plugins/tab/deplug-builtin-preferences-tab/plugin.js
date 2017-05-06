import { Plugin } from 'deplug'
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
    return [
      <div>
      <h1>Plugin Settings</h1>
      <h2>User Plugins</h2>
      <div>
        {
          this.plugins.filter((plugin) => !plugin.builtin).map((plugin) => {
            return m(PluginCard, {
              pkg: plugin.pkg,
              installed: true,
              options: plugin.options
            })
          })
        }
      </div>
      <h2>Built-in Plugins</h2>
      <div>
        {
          this.plugins.filter((plugin) => plugin.builtin).map((plugin) => {
            return m(PluginCard, {
              pkg: plugin.pkg,
              installed: true,
              options: plugin.options
            })
          })
        }
      </div>
      </div>
    ]
  }
}
