import {Config} from 'deplug'
import m from 'mithril'
import objpath from 'object-path'

export default class General {
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
    ]
  }
}
