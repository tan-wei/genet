import { Plugin } from 'deplug'
import { Deplugin } from 'deplugin'
import m from 'mithril'

export default class InstallView {
  constructor() {
    this.packages = [
      {'name': 'Now Loading...'}
    ]
  }

  oncreate() {
    let deplugin = new Deplugin()
    deplugin.cache().then((packages) => {
      this.packages = packages
      m.redraw()
      return deplugin.search()
    }).then((packages) => {
      this.packages = packages
      m.redraw()
    })
  }

  view(vnode) {
    return [
      <h1>Install Plugins </h1>
      ,
      <table>
        {
          this.packages.map((pkg) => {
            return <tr><td>{ pkg.name }</td><td></td></tr>
          })
        }
      </table>
    ]
  }
}
