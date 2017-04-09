import { Config, Theme } from 'deplug'
import m from 'mithril'
import objpath from 'object-path'

export default class GeneralView {
  constructor() {
    this.version =
      objpath.get(Config.deplug, 'version', 'n/a')
    this.electronVersion =
      objpath.get(Config.deplug, 'devDependencies.electron-deplug', 'n/a')
  }

  view(vnode) {
    return [
      <h1>General Settings </h1>
      ,
      <table>
        <tr><td>Deplug version</td><td>{ this.version }</td></tr>
        <tr><td>Electron version</td><td>{ this.electronVersion } (Custom build)</td></tr>
        <tr><td>Theme</td><td>
          <select>
          {
            Object.keys(Theme.registry).map((key) => {
              let theme = Theme.registry[key]
              return <option id={ theme.id }>{ theme.name }</option>
            })
          }
          </select>
          <p><small>Changing this option does not affect already opened tabs.</small></p>
        </td></tr>
      </table>
    ]
  }
}
