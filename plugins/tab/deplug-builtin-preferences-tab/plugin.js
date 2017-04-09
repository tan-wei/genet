import { Plugin } from 'deplug'
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
      <h1>Plugin Settings </h1>
      ,
      <table>
        {
          this.plugins.map((plugin) => {
            let name = plugin.pkg.name.replace(/^deplug-builtin-/, '')
            return <tr><td>{ name }</td><td>
              <label><input type="checkbox" name="enabled"></input>Enabled</label>
            </td></tr>
          })
        }
      </table>
    ]
  }
}
