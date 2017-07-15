import { Config } from 'deplug'
import m from 'mithril'
import objpath from 'object-path'

export default class AboutView {
  constructor() {
    this.version =
      objpath.get(Config.deplug, 'version', 'n/a')
    this.electronVersion =
      objpath.get(Config.deplug, 'devDependencies.negatron', 'n/a')
  }

  view(vnode) {
    return [
      <h1>About Deplug</h1>
      ,
      <table>
        <tr><td>Deplug version</td><td>{ this.version }</td></tr>
        <tr><td>Negatron version</td><td>{ this.electronVersion }</td></tr>
      </table>
    ]
  }
}
