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
    let page = location.hash.replace(/^#!/, '') || '/'
    return [
      <nav>
        <a
          href="#!/"
          isactive={ page==='/' }
        >General</a>
        <a
          href="#!/plugin"
          isactive={ page==='/plugin' }
        >Plugin</a>
      </nav>
      ,
      <main>
        <h1>General Settings</h1>
          <h2>General Settings</h2>
            <h3>General Settings</h3>
              <h4>General Settings</h4>
                <h5>General Settings</h5>
        <table>
          <tr><td>Deplug version: </td><td>{ this.version }</td></tr>
          <tr><td>Electron version: </td><td>{ this.electronVersion }</td></tr>
          <tr><td>Deplug version: </td><td>{ this.version }</td></tr>
          <tr><td>Electron version: </td><td>{ this.electronVersion }</td></tr>
          <tr><td>Deplug version: </td><td>{ this.version }</td></tr>
          <tr><td>Electron version: </td><td>{ this.electronVersion }</td></tr>
          <tr><td>Deplug version: </td><td>{ this.version }</td></tr>
          <tr><td>Electron version: </td><td>{ this.electronVersion }</td></tr>
          <tr><td>Deplug version: </td><td>{ this.version }</td></tr>
          <tr><td>Electron version: </td><td>{ this.electronVersion }</td></tr>
          <tr><td>Deplug version: </td><td>{ this.version }</td></tr>
          <tr><td>Electron version: </td><td>{ this.electronVersion }</td></tr>
          <tr><td>Deplug version: </td><td>{ this.version }</td></tr>
          <tr><td>Electron version: </td><td>{ this.electronVersion }</td></tr>
          <tr><td>Deplug version: </td><td>{ this.version }</td></tr>
          <tr><td>Electron version: </td><td>{ this.electronVersion }</td></tr>
          <tr><td>Deplug version: </td><td>{ this.version }</td></tr>
          <tr><td>Electron version: </td><td>{ this.electronVersion }</td></tr>
        </table>
      </main>
    ]
  }
}
