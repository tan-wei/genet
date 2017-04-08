import {Config} from 'deplug'
import m from 'mithril'
import objpath from 'object-path'

export default class View {
  constructor() {
    this.version =
      objpath.get(Config.deplug, 'version', 'n/a')
    this.electronVersion =
      objpath.get(Config.deplug, 'devDependencies.electron-deplug', 'n/a')
  }

  view(vnode) {
    return <table>
      <tr><td>Deplug version: </td><td>{ this.version }</td></tr>
      <tr><td>Electron version: </td><td>{ this.electronVersion }</td></tr>
    </table>
  }
}
